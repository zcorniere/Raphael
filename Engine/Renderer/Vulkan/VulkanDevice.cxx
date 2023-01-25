#include "Engine/Renderer/Vulkan/VulkanDevice.hxx"

#include "Engine/Renderer/Vulkan/VulkanGenericPlatform.hxx"
#include "Engine/Renderer/Vulkan/VulkanLoader.hxx"
#include "Engine/Renderer/Vulkan/VulkanUtils.hxx"

#include "vulkan/vk_enum_string_helper.h"
#define VK_TYPE_TO_STRING(Type, Value) string_##Type(Value)
#define VK_FLAGS_TO_STRING(Type, Value) string_##Type(Value).c_str()

static constexpr const char *VulkanVendorIDToString(std::uint32_t vendorID)
{
    switch (vendorID) {
        case 0x10DE: return "NVIDIA";
        case 0x1002: return "AMD";
        case 0x8086: return "INTEL";
    }
    return "Unknown";
}

static constexpr std::string GetQueueInfoString(const VkQueueFamilyProperties &Props)
{
    std::string Info;
    if ((Props.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT) { Info += " Graphics"; }
    if ((Props.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT) { Info += " Compute"; }
    if ((Props.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT) { Info += " Transfert"; }
    return Info;
};

static const std::vector<const char *> DefaultDeviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                                               VK_EXT_VALIDATION_CACHE_EXTENSION_NAME};

namespace Raphael::RHI
{

VulkanDevice::VulkanDevice(VulkanDynamicRHI *InRHI, VkPhysicalDevice InGpu)
    : Device(VK_NULL_HANDLE),
      Gpu(InGpu),
      GraphicsQueue(nullptr),
      ComputeQueue(nullptr),
      TransferQueue(nullptr),
      PresentQueue(nullptr)
{
    RHI = InRHI;

    std::memset(&GpuProps, 0, sizeof(GpuProps));
    std::memset(&PhysicalFeatures, 0, sizeof(PhysicalFeatures));

    VulkanAPI::vkGetPhysicalDeviceProperties(Gpu, &GpuProps);
    LOG(LogVulkanRHI, Info, "- DeviceName: {}", GpuProps.deviceName);
    LOG(LogVulkanRHI, Info, "- API={}.{}.{} ({:#x}) Driver={:#x} VendorId={:#x} ({})",
        VK_VERSION_MAJOR(GpuProps.apiVersion), VK_VERSION_MINOR(GpuProps.apiVersion),
        VK_VERSION_PATCH(GpuProps.apiVersion), GpuProps.apiVersion, GpuProps.driverVersion, GpuProps.vendorID,
        VulkanVendorIDToString(GpuProps.vendorID));
    LOG(LogVulkanRHI, Info, "- DeviceID={:#x} Type={}", GpuProps.deviceID,
        VK_TYPE_TO_STRING(VkPhysicalDeviceType, GpuProps.deviceType));
    LOG(LogVulkanRHI, Info, "- Max Descriptor Sets Bound {}, Timestamps {}", GpuProps.limits.maxBoundDescriptorSets,
        GpuProps.limits.timestampComputeAndGraphics);
}

VulkanDevice::~VulkanDevice()
{
    if (Device != VK_NULL_HANDLE) {
        Destroy();
        Device = VK_NULL_HANDLE;
    }
}

void VulkanDevice::InitGPU()
{
    std::uint32_t QueueCount = 0;
    VulkanAPI::vkGetPhysicalDeviceQueueFamilyProperties(Gpu, &QueueCount, nullptr);
    check(QueueCount >= 1);

    QueueFamilyProps.resize(QueueCount);
    VulkanAPI::vkGetPhysicalDeviceQueueFamilyProperties(Gpu, &QueueCount, QueueFamilyProps.data());

    // Query base features
    VulkanAPI::vkGetPhysicalDeviceFeatures(Gpu, &PhysicalFeatures);

    // Setup layers and extensions
    std::vector<const char *> DeviceExtensions = DefaultDeviceExtensions;
    VulkanPlatform::GetDeviceExtensions(this, DeviceExtensions);

    LOG(LogVulkanRHI, Info, "Device properties: Geometry {}", PhysicalFeatures.geometryShader);

    CreateDevice({}, DeviceExtensions);
}

void VulkanDevice::CreateDevice(const std::vector<const char *> &DeviceLayers,
                                const std::vector<const char *> &DeviceExtensions)
{
    VkDeviceCreateInfo DeviceInfo;
    ZeroVulkanStruct(DeviceInfo, VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);

    DeviceInfo.enabledExtensionCount = DeviceExtensions.size();
    DeviceInfo.ppEnabledExtensionNames = DeviceExtensions.data();

    DeviceInfo.enabledLayerCount = DeviceLayers.size();
    DeviceInfo.ppEnabledLayerNames = (DeviceInfo.enabledLayerCount > 0) ? (DeviceLayers.data()) : (nullptr);

    std::vector<VkDeviceQueueCreateInfo> QueueFamilyInfos;
    std::int32_t GraphicsQueueFamilyIndex = -1;
    std::int32_t ComputeQueueFamilyIndex = -1;
    std::int32_t TransferQueueFamilyIndex = -1;
    LOG(LogVulkanRHI, Info, "Found {} Queue Families", QueueFamilyProps.size());

    std::uint32_t NumPriorities = 0;
    for (std::int32_t FamilyIndex = 0; FamilyIndex < (std::int32_t)QueueFamilyProps.size(); FamilyIndex++) {
        const VkQueueFamilyProperties &CurrProps = QueueFamilyProps[FamilyIndex];

        bool bIsValidQueue = false;
        if ((CurrProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT) {
            if (GraphicsQueueFamilyIndex == -1) {
                GraphicsQueueFamilyIndex = FamilyIndex;
                bIsValidQueue = true;
            }
        }

        if ((CurrProps.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT) {
            if (GraphicsQueueFamilyIndex != FamilyIndex) {
                ComputeQueueFamilyIndex = FamilyIndex;
                bIsValidQueue = true;
            }
        }

        if ((CurrProps.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT) {
            if (TransferQueueFamilyIndex == -1 &&
                (CurrProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) != VK_QUEUE_GRAPHICS_BIT &&
                (CurrProps.queueFlags & VK_QUEUE_COMPUTE_BIT) != VK_QUEUE_COMPUTE_BIT) {
                TransferQueueFamilyIndex = FamilyIndex;
                bIsValidQueue = true;
            }
        }

        if (!bIsValidQueue) {
            LOG(LogVulkanRHI, Info, "Skipping unnecessary Queue Family {}: {} queues{}", FamilyIndex,
                CurrProps.queueCount, GetQueueInfoString(CurrProps));
            continue;
        }

        std::int32_t QueueIndex = QueueFamilyInfos.size();
        QueueFamilyInfos.resize(1 + QueueFamilyInfos.size());

        VkDeviceQueueCreateInfo &CurrQueue = QueueFamilyInfos[QueueIndex];
        ZeroVulkanStruct(CurrQueue, VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO);
        CurrQueue.queueFamilyIndex = FamilyIndex;
        CurrQueue.queueCount = CurrProps.queueCount;
        NumPriorities += CurrProps.queueCount;

        LOG(LogVulkanRHI, Info, "Initializing Queue Family {}: {} queues{}", FamilyIndex, CurrProps.queueCount,
            GetQueueInfoString(CurrProps));
    }
    std::vector<float> QueuePriorities(NumPriorities);
    float *CurrentPriorities = QueuePriorities.data();

    for (std::int32_t Index = 0; Index < (std::int32_t)QueueFamilyInfos.size(); Index++) {
        VkDeviceQueueCreateInfo &CurrQueue = QueueFamilyInfos[Index];
        CurrQueue.pQueuePriorities = CurrentPriorities;

        const VkQueueFamilyProperties &CurrProps = QueueFamilyProps[CurrQueue.queueFamilyIndex];
        for (std::uint32_t QueueIndex = 0; QueueIndex < CurrProps.queueCount; QueueIndex++) {
            *CurrentPriorities++ = 1.0f;
        }
    }

    DeviceInfo.queueCreateInfoCount = QueueFamilyInfos.size();
    DeviceInfo.pQueueCreateInfos = QueueFamilyInfos.data();

    VkResult Result = VulkanAPI::vkCreateDevice(Gpu, &DeviceInfo, nullptr, &Device);
    if (Result == VK_ERROR_INITIALIZATION_FAILED) {
        LOG(LogVulkanRHI, Fatal, "Vulkan failed to create device!");
        _exit(1);
    }
    VK_CHECK_RESULT_EXPANDED(Result);

    GraphicsQueue = new VulkanQueue(this, GraphicsQueueFamilyIndex);

    if (ComputeQueueFamilyIndex == -1) { ComputeQueueFamilyIndex = GraphicsQueueFamilyIndex; }
    ComputeQueue = new VulkanQueue(this, ComputeQueueFamilyIndex);

    if (TransferQueueFamilyIndex == -1) { TransferQueueFamilyIndex = ComputeQueueFamilyIndex; }
    TransferQueue = new VulkanQueue(this, TransferQueueFamilyIndex);

    LOG(LogVulkanRHI, Info, "Using {} device layers{}", DeviceLayers.size(), DeviceLayers.size() ? ":" : ".");
    for (const char *Layer: DeviceLayers) {
        LOG(LogVulkanRHI, Info, "* {}", Layer);
    }

    LOG(LogVulkanRHI, Info, "Using {} device extensions:", DeviceExtensions.size());
    for (const char *Extension: DeviceExtensions) {
        LOG(LogVulkanRHI, Info, "* {}", Extension);
    }
}

void VulkanDevice::PrepareForDestroy()
{
}

void VulkanDevice::Destroy()
{
    delete TransferQueue;
    delete ComputeQueue;
    delete GraphicsQueue;

    VulkanAPI::vkDestroyDevice(Device, nullptr);
    Device = VK_NULL_HANDLE;
}

void VulkanDevice::WaitUntilIdle()
{
    VK_CHECK_RESULT(VulkanAPI::vkDeviceWaitIdle(Device));
}

}    // namespace Raphael::RHI

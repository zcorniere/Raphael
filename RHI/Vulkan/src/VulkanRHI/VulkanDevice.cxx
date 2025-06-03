#include "VulkanRHI/VulkanDevice.hxx"

#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanLoader.hxx"
#include "VulkanRHI/VulkanMemoryManager.hxx"
#include "VulkanRHI/VulkanPlatform.hxx"
#include "VulkanRHI/VulkanQueue.hxx"
#include "VulkanRHI/VulkanSynchronization.hxx"
#include "VulkanRHI/VulkanUtils.hxx"

#include "Engine/Misc/Utils.hxx"
#include "Engine/Platforms/PlatformMisc.hxx"

static constexpr const char* VulkanVendorIDToString(std::uint32_t vendorID)
{
    switch (vendorID)
    {
        case 0x10DE:
            return "NVIDIA";
        case 0x1002:
            return "AMD";
        case 0x8086:
            return "INTEL";
    }
    return "Unknown";
}

static constexpr std::string GetQueueInfoString(const VkQueueFamilyProperties& Props)
{
    std::string Info;
    if ((Props.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
    {
        Info += " Graphics";
    }
    if ((Props.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT)
    {
        Info += " Compute";
    }
    if ((Props.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT)
    {
        Info += " Transfert";
    }
    return Info;
};

namespace VulkanRHI
{

FVulkanDevice::FVulkanDevice(VkPhysicalDevice InGpu)
    : GraphicsQueue(nullptr)
    , ComputeQueue(nullptr)
    , TransferQueue(nullptr)
    , PresentQueue(nullptr)
    , MemoryAllocator(nullptr)
    , Device(VK_NULL_HANDLE)
    , Gpu(InGpu)
{
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

FVulkanDevice::~FVulkanDevice()
{
    if (Device != VK_NULL_HANDLE)
    {
        Destroy();
    }
}

void FVulkanDevice::SetName(std::string_view InName)
{
    FNamedClass::SetName(InName);
    if (Device)
    {
        VULKAN_SET_DEBUG_NAME(this, VK_OBJECT_TYPE_DEVICE, Device, "{:s}", InName);
        if (Gpu)
        {
            VULKAN_SET_DEBUG_NAME(this, VK_OBJECT_TYPE_PHYSICAL_DEVICE, Gpu, "{:s}.Physical", InName);
        }
    }
}

std::string_view FVulkanDevice::GetDeviceName() const
{
    return GpuProps.deviceName;
}

void FVulkanDevice::InitPhysicalDevice()
{
    std::uint32_t QueueCount = 0;
    VulkanAPI::vkGetPhysicalDeviceQueueFamilyProperties(Gpu, &QueueCount, nullptr);
    check(QueueCount >= 1);

    QueueFamilyProps.Resize(QueueCount);
    VulkanAPI::vkGetPhysicalDeviceQueueFamilyProperties(Gpu, &QueueCount, QueueFamilyProps.Raw());

    // Query base features
    VulkanAPI::vkGetPhysicalDeviceFeatures(Gpu, &PhysicalFeatures);

    // Setup layers and extensions
    FVulkanDeviceExtensionArray DeviceExtensions = GetVulkanDynamicRHI()->GetVulkanPlatform().GetDeviceExtensions();
    if (!CreateDeviceAndQueue({}, DeviceExtensions))
    {
        LOG(LogVulkanRHI, Fatal, "Failed to create Vulkan device and queues! Exiting...");
        Utils::RequestExit(1, true);
    }

    MemoryAllocator = std::make_unique<FVulkanMemoryManager>(this);

    ImmediateContext = static_cast<FVulkanCommandContext*>(RHI::Get()->RHIGetCommandContext());
}

bool FVulkanDevice::CreateDeviceAndQueue(const TArray<const char*>& DeviceLayers,
                                         const FVulkanDeviceExtensionArray& Extensions)
{
    VkPhysicalDeviceFeatures EnabledFeature{
        .fillModeNonSolid = VK_TRUE,
    };
    VkPhysicalDeviceShaderDrawParametersFeatures ShaderDrawParameters{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES,
        .pNext = nullptr,
        .shaderDrawParameters = VK_TRUE,
    };
    VkDeviceCreateInfo DeviceInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &ShaderDrawParameters,
        .flags = 0,
        .enabledLayerCount = DeviceLayers.Size(),
        .ppEnabledLayerNames = DeviceLayers.Raw(),
        .pEnabledFeatures = &EnabledFeature,
    };

    TArray<const char*> DeviceExtensions;

    TArray<VkExtensionProperties> QueringDeviceExtensions =
        GetVulkanDynamicRHI()->GetVulkanPlatform().GetDriverSupportedDeviceExtensions(Gpu, nullptr);

    for (const std::unique_ptr<IDeviceVulkanExtension>& Extension: Extensions)
    {
        if (QueringDeviceExtensions.FindByLambda(
                [&Extension](const VkExtensionProperties& Prop)
                { return std::strcmp(Extension->GetExtensionName(), Prop.extensionName) == 0; }) == nullptr)
        {
            if (Extension->IsExtensionRequired())
            {
                LOG(LogVulkanRHI, Error, "Missing Required Extension: {}", Extension->GetExtensionName());
                return false;
            }
            else
            {
                LOG(LogVulkanRHI, Warning, "Missing Extension: {}", Extension->GetExtensionName());
                Extension->SetSupported(false);
            }
        }
        else
        {
            Extension->PreDeviceCreated(DeviceInfo);
            DeviceExtensions.Add(Extension->GetExtensionName());
        }
    }
    DeviceInfo.enabledExtensionCount = DeviceExtensions.Size();
    DeviceInfo.ppEnabledExtensionNames = DeviceExtensions.Raw();

    TArray<VkDeviceQueueCreateInfo> QueueFamilyInfos;
    int32 GraphicsQueueFamilyIndex = -1;
    int32 ComputeQueueFamilyIndex = -1;
    int32 TransferQueueFamilyIndex = -1;
    LOG(LogVulkanRHI, Info, "Found {} Queue Families", QueueFamilyProps.Size());

    std::uint32_t NumPriorities = 0;
    for (int32 FamilyIndex = 0; FamilyIndex < (int32)QueueFamilyProps.Size(); FamilyIndex++)
    {
        const VkQueueFamilyProperties& CurrProps = QueueFamilyProps[FamilyIndex];

        bool bIsValidQueue = false;
        if ((CurrProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
        {
            if (GraphicsQueueFamilyIndex == -1)
            {
                GraphicsQueueFamilyIndex = FamilyIndex;
                bIsValidQueue = true;
            }
        }

        if ((CurrProps.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT)
        {
            if (GraphicsQueueFamilyIndex != FamilyIndex)
            {
                ComputeQueueFamilyIndex = FamilyIndex;
                bIsValidQueue = true;
            }
        }

        if ((CurrProps.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT)
        {
            if (TransferQueueFamilyIndex == -1 &&
                (CurrProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) != VK_QUEUE_GRAPHICS_BIT &&
                (CurrProps.queueFlags & VK_QUEUE_COMPUTE_BIT) != VK_QUEUE_COMPUTE_BIT)
            {
                TransferQueueFamilyIndex = FamilyIndex;
                bIsValidQueue = true;
            }
        }

        if (!bIsValidQueue)
        {
            LOG(LogVulkanRHI, Info, "Skipping unnecessary Queue Family {:d}: {:d} queues{:s}", FamilyIndex,
                CurrProps.queueCount, GetQueueInfoString(CurrProps));
            continue;
        }

        int32 QueueIndex = QueueFamilyInfos.Size();
        QueueFamilyInfos.Resize(1 + QueueFamilyInfos.Size());

        VkDeviceQueueCreateInfo& CurrQueue = QueueFamilyInfos[QueueIndex];
        CurrQueue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        CurrQueue.queueFamilyIndex = FamilyIndex;
        CurrQueue.queueCount = CurrProps.queueCount;

        NumPriorities += CurrProps.queueCount;

        LOG(LogVulkanRHI, Info, "Initializing Queue Family {:d}: {:d} queues{:s}", FamilyIndex, CurrProps.queueCount,
            GetQueueInfoString(CurrProps));
    }
    TArray<float> QueuePriorities(NumPriorities);
    float* CurrentPriorities = QueuePriorities.Raw();

    for (int32 Index = 0; Index < (int32)QueueFamilyInfos.Size(); Index++)
    {
        VkDeviceQueueCreateInfo& CurrQueue = QueueFamilyInfos[Index];
        CurrQueue.pQueuePriorities = CurrentPriorities;

        const VkQueueFamilyProperties& CurrProps = QueueFamilyProps[CurrQueue.queueFamilyIndex];
        for (std::uint32_t QueueIndex = 0; QueueIndex < CurrProps.queueCount; QueueIndex++)
        {
            *CurrentPriorities = 1.0f;
            CurrentPriorities++;
        }
    }

    DeviceInfo.queueCreateInfoCount = QueueFamilyInfos.Size();
    DeviceInfo.pQueueCreateInfos = QueueFamilyInfos.Raw();

    VkResult Result = VulkanAPI::vkCreateDevice(Gpu, &DeviceInfo, VULKAN_CPU_ALLOCATOR, &Device);
    switch (Result)
    {
        case VK_ERROR_INITIALIZATION_FAILED:
        {
            LOG(LogVulkanRHI, Fatal, "Vulkan failed to create device!");
            return false;
        }
        case VK_ERROR_EXTENSION_NOT_PRESENT:
        {
            LOG(LogVulkanRHI, Fatal, "Vulkan failed to create device due to missing extensions!");
            return false;
        }
        default:
            VK_CHECK_RESULT_EXPANDED(Result);
            break;
    }

    for (const std::unique_ptr<IDeviceVulkanExtension>& Extension: Extensions)
    {
        Extension->PostDeviceCreated(ExtensionStatus);
    }

    GraphicsQueue = std::make_unique<FVulkanQueue>(this, GraphicsQueueFamilyIndex);
    GraphicsQueue->SetName("Graphics Queue");

    if (ComputeQueueFamilyIndex == -1)
    {
        ComputeQueueFamilyIndex = GraphicsQueueFamilyIndex;
    }
    ComputeQueue = std::make_unique<FVulkanQueue>(this, ComputeQueueFamilyIndex);
    ComputeQueue->SetName("Compute Queue");

    if (TransferQueueFamilyIndex == -1)
    {
        TransferQueueFamilyIndex = ComputeQueueFamilyIndex;
    }
    TransferQueue = std::make_unique<FVulkanQueue>(this, TransferQueueFamilyIndex);
    TransferQueue->SetName("Transfer Queue");

    LOG(LogVulkanRHI, Info, "Using {} device layers{}", DeviceLayers.Size(), DeviceLayers.Size() ? ":" : ".");
    for (const char* Layer: DeviceLayers)
    {
        LOG(LogVulkanRHI, Info, "* {}", Layer);
    }

    LOG(LogVulkanRHI, Info, "Using {} device extensions:", DeviceExtensions.Size());
    for (const char* Extension: DeviceExtensions)
    {
        LOG(LogVulkanRHI, Info, "* {}", Extension);
    }
    return true;
}

static bool DoesQueueSupportPresent(VkSurfaceKHR Surface, VkPhysicalDevice PhysicalDevice, FVulkanQueue* Queue)
{
    VkBool32 bSupportsPresent = VK_FALSE;
    const uint32 FamilyIndex = Queue->GetFamilyIndex();
    VK_CHECK_RESULT(
        VulkanAPI::vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, FamilyIndex, Surface, &bSupportsPresent));
    if (bSupportsPresent)
    {
        LOG(LogVulkanRHI, Info, "Queue Family {}({}): Supports Present", FamilyIndex, Queue->GetName());
    }
    return bSupportsPresent == VK_TRUE;
}

void FVulkanDevice::SetupPresentQueue(VkSurfaceKHR Surface)
{
    if (!PresentQueue)
    {
        if (!DoesQueueSupportPresent(Surface, Gpu, GraphicsQueue.get()))
        {
            FPlatformMisc::DisplayMessageBox(
                EBoxMessageType::Ok, "Cannot find a compatible Vulkan device that supports surface presentation.\n\n",
                "Vulkan device not available");
            Utils::RequestExit(1);
        }

        if (TransferQueue->GetFamilyIndex() != GraphicsQueue->GetFamilyIndex() &&
            TransferQueue->GetFamilyIndex() != ComputeQueue->GetFamilyIndex())
        {
            DoesQueueSupportPresent(Surface, Gpu, TransferQueue.get());
        }

        const bool bCompute = DoesQueueSupportPresent(Surface, Gpu, ComputeQueue.get());
        if (ComputeQueue->GetFamilyIndex() != GraphicsQueue->GetFamilyIndex() && bCompute)
        {
            PresentQueue = ComputeQueue.get();
        }
        else
        {
            PresentQueue = GraphicsQueue.get();
        }
        LOG(LogVulkanRHI, Info, "Using {} as the present Queue", PresentQueue->GetName());
    }
}

void FVulkanDevice::Destroy()
{
    WaitUntilIdle();

    MemoryAllocator.reset();

    GraphicsQueue = nullptr;
    ComputeQueue = nullptr;
    TransferQueue = nullptr;

    // Present Queue is a copy
    PresentQueue = nullptr;

    if (Device)
    {
        VulkanAPI::vkDestroyDevice(Device, VULKAN_CPU_ALLOCATOR);
        Device = VK_NULL_HANDLE;
    }
}

void FVulkanDevice::WaitUntilIdle()
{
    VK_CHECK_RESULT(VulkanAPI::vkDeviceWaitIdle(Device));
}

}    // namespace VulkanRHI

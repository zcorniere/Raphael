#include "Engine/Renderer/Vulkan/VulkanDynamicRHI.hxx"

#include "Engine/Renderer/Vulkan/VulkanDevice.hxx"
#include "Engine/Renderer/Vulkan/VulkanGenericPlatform.hxx"
#include "Engine/Renderer/Vulkan/VulkanLoader.hxx"
#include "Engine/Renderer/Vulkan/VulkanUtils.hxx"

namespace VulkanRHI
{

VulkanDynamicRHI::VulkanDynamicRHI(): m_Instance(VK_NULL_HANDLE), Device(nullptr)
{
    LOG(LogVulkanRHI, Info, "Built with Vulkan header version {}.{}.{}",
        VK_API_VERSION_MAJOR(VK_HEADER_VERSION_COMPLETE), VK_API_VERSION_MINOR(VK_HEADER_VERSION_COMPLETE),
        VK_API_VERSION_PATCH(VK_HEADER_VERSION_COMPLETE));

    if (!VulkanPlatform::LoadVulkanLibrary()) {
        LOG(LogVulkanRHI, Fatal,
            "Failed to find all of the required Vulkan entry points; make sure your driver supports Vulkan!");
    }

    CreateInstance();
    SelectDevice();

    GDynamicRHI = this;
}

VulkanDynamicRHI::~VulkanDynamicRHI()
{
}

VkInstance VulkanDynamicRHI::RHIGetVkInstance() const
{
    return GetInstance();
}

VkDevice VulkanDynamicRHI::RHIGetVkDevice() const
{
    return Device->GetInstanceHandle();
}

VkPhysicalDevice VulkanDynamicRHI::RHIGetVkPhysicalDevice() const
{
    return Device->GetPhysicalHandle();
}

void VulkanDynamicRHI::Init()
{
    Device->InitGPU();
}

void VulkanDynamicRHI::PostInit()
{
}

void VulkanDynamicRHI::Shutdown()
{
    Device->PrepareForDestroy();
    Device->Destroy();

    Device = nullptr;
    Devices.clear();

#if VULKAN_DEBUGGING_ENABLED
    RemoveDebugLayerCallback();
#endif

    VulkanAPI::vkDestroyInstance(m_Instance, nullptr);
    VulkanPlatform::FreeVulkanLibrary();
}

Ref<VulkanDevice> VulkanDynamicRHI::GetDevice()
{
    return Device;
}

void VulkanDynamicRHI::CreateInstance()
{
    VkApplicationInfo AppInfo;
    ZeroVulkanStruct(AppInfo, VK_STRUCTURE_TYPE_APPLICATION_INFO);
    AppInfo.pEngineName = "RaphaelEngine";
    AppInfo.apiVersion = RHI_VULKAN_VERSION;

    VkInstanceCreateInfo InstInfo;
    ZeroVulkanStruct(InstInfo, VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
    InstInfo.pApplicationInfo = &AppInfo;

    // TODO: Wrap it into its own class ?
    std::vector<const char *> VulkanExtensions{VK_KHR_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
    VulkanPlatform::GetInstanceExtensions(VulkanExtensions);

    std::vector<const char *> ValidationLayers{"VK_LAYER_KHRONOS_validation"};

    InstInfo.enabledExtensionCount = VulkanExtensions.size();
    InstInfo.ppEnabledExtensionNames = VulkanExtensions.data();

    InstInfo.enabledLayerCount = ValidationLayers.size();
    InstInfo.ppEnabledLayerNames = ValidationLayers.data();

    VkResult Result = VulkanAPI::vkCreateInstance(&InstInfo, nullptr, &m_Instance);

    if (Result == VK_ERROR_INCOMPATIBLE_DRIVER) {
        LOG(LogVulkanRHI, Fatal, "Cannot find a compatible Vulkan driver.");
        _exit(1);
    } else if (Result == VK_ERROR_EXTENSION_NOT_PRESENT) {
        // TODO: print missing extension
        LOG(LogVulkanRHI, Fatal, "Extension not found !");
        _exit(1);
    } else if (Result != VK_SUCCESS) {
        LOG(LogVulkanRHI, Fatal, "Vulkan failed to create instance!");
        _exit(1);
    }

    VK_CHECK_RESULT(Result);

    if (!VulkanPlatform::LoadVulkanInstanceFunctions(m_Instance)) {
        LOG(LogVulkanRHI, Fatal, "Couldn't find some of Vulkan's entry points !");
    }

#if VULKAN_DEBUGGING_ENABLED
    SetupDebugLayerCallback();
#endif
}

void VulkanDynamicRHI::SelectDevice()
{
    std::uint32_t GpuCount = 0;
    VkResult Result = VulkanAPI::vkEnumeratePhysicalDevices(m_Instance, &GpuCount, nullptr);
    if (Result == VK_ERROR_INITIALIZATION_FAILED) {
        LOG(LogVulkanRHI, Fatal, "Vulkan failed to find enumerate device!");
        _exit(1);
    }
    VK_CHECK_RESULT_EXPANDED(Result);
    checkMsg(GpuCount >= 1, "No GPU(s)/Driver(s) that support Vulkan were found!");

    std::vector<VkPhysicalDevice> PhysicalDevices(GpuCount);
    VK_CHECK_RESULT_EXPANDED(VulkanAPI::vkEnumeratePhysicalDevices(m_Instance, &GpuCount, PhysicalDevices.data()));
    checkMsg(GpuCount >= 1, "Couldn't enumerate physical devices!");

    struct DeviceInfo {
        Ref<VulkanDevice> Device;
        std::uint32_t DeviceIndex;
    };
    std::vector<DeviceInfo> DiscreteDevice;
    std::vector<DeviceInfo> IntegratedDevice;

    LOG(LogVulkanRHI, Info, "Found {} device(s)", GpuCount);
    for (std::uint32_t Index = 0; Index < GpuCount; Index++) {
        LOG(LogVulkanRHI, Info, "Device {}:", Index);
        Ref<VulkanDevice> NewDevice = Ref<VulkanDevice>::Create(this, PhysicalDevices[Index]);
        Devices.push_back(NewDevice);

        const bool bIsDiscrete = (NewDevice->GetDeviceProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
        const bool bIsCPUDevice = (NewDevice->GetDeviceProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU);

        if (bIsDiscrete) {
            DiscreteDevice.push_back({NewDevice, Index});
        } else if (bIsCPUDevice) {
            LOG(LogVulkanRHI, Info, "Skipping device[{}] of type VK_PHYSICAL_DEVICE_TYPE_CPU",
                NewDevice->GetDeviceProperties().deviceName);
        } else {
            IntegratedDevice.push_back({NewDevice, Index});
        }
    }

    std::uint32_t DeviceIndex = -1;
    DiscreteDevice.insert(DiscreteDevice.end(), IntegratedDevice.begin(), IntegratedDevice.end());

    if (DiscreteDevice.size() > 0) {
        Device = DiscreteDevice[0].Device;
        DeviceIndex = DiscreteDevice[0].DeviceIndex;
    } else {
        LOG(LogVulkanRHI, Info, "Cannot find compatible Vulkan device");
        _exit(1);
    }

    LOG(LogVulkanRHI, Info, "Chosen device index: {}", DeviceIndex);
}

}    // namespace VulkanRHI

#include "Engine/Renderer/Vulkan/VulkanRHI.hxx"

#include "Engine/Platforms/PlatformMisc.hxx"
#include "Engine/Renderer/Vulkan/VulkanDevice.hxx"
#include "Engine/Renderer/Vulkan/VulkanPlatform.hxx"
#include "Engine/Renderer/Vulkan/VulkanLoader.hxx"
#include "Engine/Renderer/Vulkan/VulkanUtils.hxx"

static std::string GetMissingExtensions(std::vector<const char *> VulkanExtensions);

namespace VulkanRHI
{

VulkanDynamicRHI::VulkanDynamicRHI(): m_Instance(VK_NULL_HANDLE), Device(nullptr)
{
    SetName("VulkanRHI");

    LOG(LogVulkanRHI, Info, "Built with Vulkan header version {}.{}.{}",
        VK_API_VERSION_MAJOR(VK_HEADER_VERSION_COMPLETE), VK_API_VERSION_MINOR(VK_HEADER_VERSION_COMPLETE),
        VK_API_VERSION_PATCH(VK_HEADER_VERSION_COMPLETE));

    if (!VulkanPlatform::LoadVulkanLibrary()) {
        PlatformMisc::DisplayMessageBox(
            EBoxMessageType::Ok,
            "Unable to load Vulkan library and/or acquire the necessary function pointers. Make sure an "
            "up-to-date libvulkan.so.1 is installed.",
            "Unable to initialize Vulkan.");
        LOG(LogVulkanRHI, Fatal,
            "Failed to find all of the required Vulkan entry points; make sure your driver supports Vulkan!");
        _exit(1);
    }

    CreateInstance();
    SelectDevice();
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
    GenericRHI::Init();

    Device->InitPhysicalDevice();
}

void VulkanDynamicRHI::PostInit()
{
}

void VulkanDynamicRHI::Shutdown()
{
    Device->PrepareForDestroy();
    Device->Destroy();
    Device = nullptr;

#if VULKAN_DEBUGGING_ENABLED
    RemoveDebugLayerCallback();
#endif

    VulkanAPI::vkDestroyInstance(m_Instance, nullptr);
    VulkanPlatform::FreeVulkanLibrary();

    GenericRHI::Shutdown();
}

Ref<VulkanDevice> VulkanDynamicRHI::GetDevice()
{
    return Device;
}

void VulkanDynamicRHI::CreateInstance()
{
    VkApplicationInfo AppInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pEngineName = "RaphaelEngine",
        .apiVersion = RHI_VULKAN_VERSION,
    };

    VkInstanceCreateInfo InstInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &AppInfo,
    };

    // TODO: Wrap it into its own class ?
    std::vector<const char *> VulkanExtensions{
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    };
    VulkanPlatform::GetInstanceExtensions(VulkanExtensions);

    std::vector<const char *> ValidationLayers{"VK_LAYER_KHRONOS_validation"};

    InstInfo.enabledExtensionCount = VulkanExtensions.size();
    InstInfo.ppEnabledExtensionNames = VulkanExtensions.data();

    InstInfo.enabledLayerCount = ValidationLayers.size();
    InstInfo.ppEnabledLayerNames = ValidationLayers.data();

    VkResult Result = VulkanAPI::vkCreateInstance(&InstInfo, nullptr, &m_Instance);

    if (Result == VK_ERROR_INCOMPATIBLE_DRIVER) {
        PlatformMisc::DisplayMessageBox(
            EBoxMessageType::Ok,
            "Unable to load Vulkan library and/or acquire the necessary function pointers. Make sure an "
            "up-to-date libvulkan.so.1 is installed.",
            "Unable to initialize Vulkan.");
        LOG(LogVulkanRHI, Fatal, "Cannot find a compatible Vulkan driver.");
        _exit(1);
    } else if (Result == VK_ERROR_EXTENSION_NOT_PRESENT) {
        std::string MissingExtensions = GetMissingExtensions(VulkanExtensions);

        PlatformMisc::DisplayMessageBox(
            EBoxMessageType::Ok,
            cpplogger::fmt::format("Vulkan driver doesn't contain specified extensions:\n{:s}\nMake sure your layers "
                                   "path is set appropriately.",
                                   MissingExtensions),
            "Incompatible Vulkan driver found!");
        LOG(LogVulkanRHI, Fatal, "Extension not found !");
        _exit(1);
    } else if (Result != VK_SUCCESS) {
        LOG(LogVulkanRHI, Fatal, "Vulkan failed to create instance!");
        PlatformMisc::DisplayMessageBox(
            EBoxMessageType::Ok,
            "Vulkan failed to create instance (apiVersion=0x%x)\n\nDo you have a compatible Vulkan "
            "driver (ICD) installed?\nPlease look at "
            "the Getting Started guide for additional information.",
            "No Vulkan driver found!");
        _exit(1);
    }

    VK_CHECK_RESULT(Result);

    if (!VulkanPlatform::LoadVulkanInstanceFunctions(m_Instance)) {
        LOG(LogVulkanRHI, Fatal, "Couldn't find some of Vulkan's entry points !");
        PlatformMisc::DisplayMessageBox(EBoxMessageType::Ok,
                                 "Failed to find all required Vulkan entry points! Try updating your driver.",
                                 "No Vulkan entry points found!");
        _exit(1);
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
        return;
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
    std::vector<Ref<VulkanDevice>> Devices;
    std::vector<DeviceInfo> DiscreteDevice;
    std::vector<DeviceInfo> IntegratedDevice;

    LOG(LogVulkanRHI, Info, "Found {} device(s)", GpuCount);
    for (std::uint32_t Index = 0; Index < GpuCount; Index++) {
        LOG(LogVulkanRHI, Info, "Device {}:", Index);
        Ref<VulkanDevice> NewDevice = Ref<VulkanDevice>::Create(PhysicalDevices[Index]);
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

    uint32 DeviceIndex = (uint32)-1;
    DiscreteDevice.insert(DiscreteDevice.end(), IntegratedDevice.begin(), IntegratedDevice.end());

    if (DiscreteDevice.size() > 0) {
        Device = DiscreteDevice[0].Device;
        DeviceIndex = DiscreteDevice[0].DeviceIndex;
    } else {
        LOG(LogVulkanRHI, Info, "Cannot find compatible Vulkan device");
        return;
    }

    LOG(LogVulkanRHI, Info, "Chosen device index: {}", DeviceIndex);
}

}    // namespace VulkanRHI

static std::string GetMissingExtensions(std::vector<const char *> VulkanExtensions)
{
    std::string MissingExtensions;
    uint32_t PropertyCount;
    VulkanRHI::VulkanAPI::vkEnumerateInstanceExtensionProperties(nullptr, &PropertyCount, nullptr);

    std::vector<VkExtensionProperties> Properties;
    Properties.resize(PropertyCount);
    VulkanRHI::VulkanAPI::vkEnumerateInstanceExtensionProperties(nullptr, &PropertyCount, Properties.data());

    for (const char *Extension: VulkanExtensions) {
        bool bExtensionFound = false;

        for (uint32_t PropertyIndex = 0; PropertyIndex < PropertyCount; PropertyIndex++) {
            const char *PropertyExtensionName = Properties[PropertyIndex].extensionName;

            if (!std::strcmp(PropertyExtensionName, Extension)) {
                bExtensionFound = true;
                break;
            }
        }

        if (!bExtensionFound) {
            LOG(LogVulkanRHI, Error, "Missing required Vulkan extension: {:s}", Extension);
            MissingExtensions += Extension;
            MissingExtensions += "\n";
        }
    }
    return MissingExtensions;
}

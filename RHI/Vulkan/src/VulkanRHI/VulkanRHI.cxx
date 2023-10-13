#include "VulkanRHI/VulkanRHI.hxx"

#include "Engine/Misc/Utils.hxx"

#include "Engine/Platforms/PlatformMisc.hxx"

#include "VulkanRHI/RenderPass/RenderPassManager.hxx"
#include "VulkanRHI/RenderPass/VulkanRenderPass.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanLoader.hxx"
#include "VulkanRHI/VulkanPlatform.hxx"
#include "VulkanRHI/VulkanShaderCompiler.hxx"
#include "VulkanRHI/VulkanUtils.hxx"

// RHI Creation Implementation
GenericRHI* RHI::CreateRHI()
{
    RPH_PROFILE_FUNC()

    GDynamicRHI = std::make_unique<VulkanRHI::VulkanDynamicRHI>();
    return GDynamicRHI.get();
}
//

static std::string GetMissingExtensions(Array<const char*> VulkanExtensions);

namespace VulkanRHI
{

VulkanDynamicRHI::VulkanDynamicRHI(): m_Instance(VK_NULL_HANDLE), Device(nullptr)
{
    LOG(LogVulkanRHI, Info, "Built with Vulkan header version {}.{}.{}",
        VK_API_VERSION_MAJOR(VK_HEADER_VERSION_COMPLETE), VK_API_VERSION_MINOR(VK_HEADER_VERSION_COMPLETE),
        VK_API_VERSION_PATCH(VK_HEADER_VERSION_COMPLETE));

    if (!ensure(VulkanPlatform::LoadVulkanLibrary())) {
        PlatformMisc::DisplayMessageBox(
            EBoxMessageType::Ok,
            "Unable to load Vulkan library and/or acquire the necessary function pointers. Make sure an "
            "up-to-date libvulkan.so.1 is installed.",
            "Unable to initialize Vulkan.");
        LOG(LogVulkanRHI, Fatal,
            "Failed to find all of the required Vulkan entry points; make sure your driver supports Vulkan!");
        Utils::RequestExit(1);
    }
}

VulkanDynamicRHI::~VulkanDynamicRHI()
{
    VulkanPlatform::FreeVulkanLibrary();
}

VkDevice VulkanDynamicRHI::RHIGetVkDevice() const
{
    return Device->GetHandle();
}

VkPhysicalDevice VulkanDynamicRHI::RHIGetVkPhysicalDevice() const
{
    return Device->GetPhysicalHandle();
}

void VulkanDynamicRHI::Init()
{
    RPH_PROFILE_FUNC()

    CreateInstance();
    SelectDevice();

    Device->InitPhysicalDevice();
    Device->SetName("Main Vulkan Device");

    RPassManager = std::make_unique<RenderPassManager>(Device.get());
    ShaderCompiler = std::make_unique<VulkanShaderCompiler>();

#if VULKAN_DEBUGGING_ENABLED
    ShaderCompiler->SetOptimizationLevel(VulkanShaderCompiler::OptimizationLevel::PerfWithDebug);
#else
    ShaderCompiler->SetOptimizationLevel(VulkanShaderCompiler::OptimizationLevel::Performance);
#endif
}

void VulkanDynamicRHI::PostInit()
{
}

void VulkanDynamicRHI::Shutdown()
{
    if (Device) {
        Device->WaitUntilIdle();
    }
    RPassManager = nullptr;

    Device = nullptr;

#if VULKAN_DEBUGGING_ENABLED
    RemoveDebugLayerCallback();
#endif

    VulkanAPI::vkDestroyInstance(m_Instance, VULKAN_CPU_ALLOCATOR);
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
    Array<const char*> InstanceExtensions
    {
        VK_KHR_SURFACE_EXTENSION_NAME,
#if VULKAN_DEBUGGING_ENABLED
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
    };
    VulkanPlatform::GetInstanceExtensions(InstanceExtensions);

    InstInfo.enabledExtensionCount = InstanceExtensions.Size();
    InstInfo.ppEnabledExtensionNames = InstanceExtensions.Raw();

#if VULKAN_DEBUGGING_ENABLED
    Array<const char*> ValidationLayers = GetSupportedInstanceLayers();

    InstInfo.enabledLayerCount = ValidationLayers.Size();
    InstInfo.ppEnabledLayerNames = ValidationLayers.Raw();
#endif

    VkResult Result = VulkanAPI::vkCreateInstance(&InstInfo, VULKAN_CPU_ALLOCATOR, &m_Instance);

    if (Result == VK_ERROR_INCOMPATIBLE_DRIVER) {
        PlatformMisc::DisplayMessageBox(
            EBoxMessageType::Ok,
            "Unable to load Vulkan library and/or acquire the necessary function pointers. Make sure an "
            "up-to-date libvulkan.so.1 is installed.",
            "Unable to initialize Vulkan.");
        LOG(LogVulkanRHI, Fatal, "Cannot find a compatible Vulkan driver.");
        Utils::RequestExit(1);
    } else if (Result == VK_ERROR_EXTENSION_NOT_PRESENT) {
        std::string MissingExtensions = GetMissingExtensions(InstanceExtensions);

        PlatformMisc::DisplayMessageBox(
            EBoxMessageType::Ok,
            std::format("Vulkan driver doesn't contain specified extensions:\n{:s}\nMake sure your layers "
                        "path is set appropriately.",
                        MissingExtensions),
            "Incompatible Vulkan driver found!");
        LOG(LogVulkanRHI, Fatal, "Extension not found : {} !", MissingExtensions);
        Utils::RequestExit(1);
    } else if (Result != VK_SUCCESS) {
        LOG(LogVulkanRHI, Fatal, "Vulkan failed to create instance! {:s}", magic_enum::enum_name(Result));
        PlatformMisc::DisplayMessageBox(EBoxMessageType::Ok,
                                        "Vulkan failed to create instance !\n\nDo you have a compatible Vulkan "
                                        "driver (ICD) installed?",
                                        "No Vulkan driver found!");
        Utils::RequestExit(1);
    }

    VK_CHECK_RESULT(Result);

    if (!VulkanPlatform::LoadVulkanInstanceFunctions(m_Instance)) {
        LOG(LogVulkanRHI, Fatal, "Couldn't find some of Vulkan's entry points !");
        PlatformMisc::DisplayMessageBox(EBoxMessageType::Ok,
                                        "Failed to find all required Vulkan entry points! Try updating your driver.",
                                        "No Vulkan entry points found!");
        Utils::RequestExit(1);
    }

    LOG(LogVulkanRHI, Info, "Using {} Instance extensions {}", InstanceExtensions.Size(),
        InstanceExtensions.Size() ? ":" : ".");
    for (const char* Layer: InstanceExtensions) {
        LOG(LogVulkanRHI, Info, "* {}", Layer);
    }

#if VULKAN_DEBUGGING_ENABLED
    LOG(LogVulkanRHI, Warning, "Vulkan Debugging is enabled {}!",
        (bValidationLayersAreMissing) ? ("but some instance layers are missing ") : (""));
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

    Array<VkPhysicalDevice> PhysicalDevices(GpuCount);
    VK_CHECK_RESULT_EXPANDED(VulkanAPI::vkEnumeratePhysicalDevices(m_Instance, &GpuCount, PhysicalDevices.Raw()));
    checkMsg(GpuCount >= 1, "Couldn't enumerate physical devices!");

    struct DeviceInfo {
        VulkanDevice* Device;
        std::uint32_t DeviceIndex;
    };
    Array<VulkanDevice*> Devices;
    Array<DeviceInfo> DiscreteDevice;
    Array<DeviceInfo> IntegratedDevice;

    LOG(LogVulkanRHI, Info, "Found {} device(s)", GpuCount);
    for (std::uint32_t Index = 0; Index < GpuCount; Index++) {
        LOG(LogVulkanRHI, Info, "Device {}:", Index);
        VulkanDevice* NewDevice = new VulkanDevice(PhysicalDevices[Index]);
        Devices.Add(NewDevice);

        const bool bIsDiscrete = (NewDevice->GetDeviceProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
        const bool bIsCPUDevice = (NewDevice->GetDeviceProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU);

        if (bIsDiscrete) {
            DiscreteDevice.Add({NewDevice, Index});
        } else if (bIsCPUDevice) {
            LOG(LogVulkanRHI, Info, "Skipping device[{}] of type VK_PHYSICAL_DEVICE_TYPE_CPU",
                NewDevice->GetDeviceProperties().deviceName);
        } else {
            IntegratedDevice.Add({NewDevice, Index});
        }
    }

    uint32 DeviceIndex = (uint32)-1;
    DiscreteDevice.Append(IntegratedDevice);

    if (DiscreteDevice.Size() > 0) {
        Device.reset(DiscreteDevice[0].Device);
        DeviceIndex = DiscreteDevice[0].DeviceIndex;
    } else if (IntegratedDevice.Size() > 0) {
        Device.reset(IntegratedDevice[0].Device);
        DeviceIndex = IntegratedDevice[0].DeviceIndex;
    } else {
        LOG(LogVulkanRHI, Info, "Cannot find compatible Vulkan device");
        return;
    }
    Devices.Remove(Device.get());
    Devices.Clear(true);

    LOG(LogVulkanRHI, Info, "Chosen device index: {}", DeviceIndex);
}

}    // namespace VulkanRHI

static std::string GetMissingExtensions(Array<const char*> VulkanExtensions)
{
    std::string MissingExtensions;
    uint32_t PropertyCount;
    VulkanRHI::VulkanAPI::vkEnumerateInstanceExtensionProperties(nullptr, &PropertyCount, nullptr);

    Array<VkExtensionProperties> Properties;
    Properties.Resize(PropertyCount);
    VulkanRHI::VulkanAPI::vkEnumerateInstanceExtensionProperties(nullptr, &PropertyCount, Properties.Raw());

    for (const char* Extension: VulkanExtensions) {
        bool bExtensionFound = false;

        for (uint32_t PropertyIndex = 0; PropertyIndex < PropertyCount; PropertyIndex++) {
            const char* PropertyExtensionName = Properties[PropertyIndex].extensionName;

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

#include "VulkanRHI/VulkanRHI.hxx"

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "Engine/Core/Engine.hxx"
#include "Engine/Misc/Utils.hxx"

#include "Engine/Platforms/PlatformMisc.hxx"

#include "VulkanRHI/Resources/VulkanViewport.hxx"

#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanLoader.hxx"
#include "VulkanRHI/VulkanPendingState.hxx"
#include "VulkanRHI/VulkanPlatform.hxx"
#include "VulkanRHI/VulkanShaderCompiler.hxx"
#include "VulkanRHI/VulkanUtils.hxx"

// RHI Creation Implementation
extern "C" FGenericRHI* RHI_CreateRHI()
{
    RPH_PROFILE_FUNC()

    return new VulkanRHI::FVulkanDynamicRHI;
}
//

static std::string GetMissingExtensions(TArray<const char*> VulkanExtensions);

namespace VulkanRHI
{

FVulkanDynamicRHI::FVulkanDynamicRHI(): Device(nullptr), ShaderCompiler(nullptr)
{
    LOG(LogVulkanRHI, Info, "Built with Vulkan header version {}.{}.{}",
        VK_API_VERSION_MAJOR(VK_HEADER_VERSION_COMPLETE), VK_API_VERSION_MINOR(VK_HEADER_VERSION_COMPLETE),
        VK_API_VERSION_PATCH(VK_HEADER_VERSION_COMPLETE));

    if (!ensure(FVulkanPlatform::LoadVulkanLibrary())) {
        FPlatformMisc::DisplayMessageBox(
            EBoxMessageType::Ok,
            "Unable to load Vulkan library and/or acquire the necessary function pointers. Make sure an "
            "up-to-date libvulkan.so.1 is installed.",
            "Unable to initialize Vulkan.");
        LOG(LogVulkanRHI, Fatal,
            "Failed to find all of the required Vulkan entry points; make sure your driver supports Vulkan!");
        Utils::RequestExit(1);
    }
}

FVulkanDynamicRHI::~FVulkanDynamicRHI()
{
    FVulkanPlatform::FreeVulkanLibrary();
}

void FVulkanDynamicRHI::Tick(float fDeltaTime)
{
    (void)fDeltaTime;

    ENQUEUE_RENDER_COMMAND(BeginFrame)([](FFRHICommandList& CommandList) { CommandList.BeginFrame(); });

    for (WeakRef<RHIScene>& Scene: ScenesContainers) {
        if (Scene.IsValid()) {
            ENQUEUE_RENDER_COMMAND(RenderScene)
            ([Scene](FFRHICommandList& CommandList) mutable { Scene->TickRenderer(CommandList); });
        }
    }

    ENQUEUE_RENDER_COMMAND(EndFrame)([](FFRHICommandList& CommandList) { CommandList.EndFrame(); });
}

VkDevice FVulkanDynamicRHI::RHIGetVkDevice() const
{
    return Device->GetHandle();
}

VkPhysicalDevice FVulkanDynamicRHI::RHIGetVkPhysicalDevice() const
{
    return Device->GetPhysicalHandle();
}

void FVulkanDynamicRHI::Init()
{
    RPH_PROFILE_FUNC()

    m_Instance = CreateInstance(DebugLayer.GetSupportedInstanceLayers());

#if VULKAN_DEBUGGING_ENABLED
    LOG(LogVulkanRHI, Warning, "Vulkan Debugging is enabled {}!",
        (DebugLayer.IsValidationLayersMissing()) ? ("but some instance layers are missing ") : (""));
    DebugLayer.SetupDebugLayer(m_Instance);
#endif

    Device.reset(SelectDevice(m_Instance));

    Device->InitPhysicalDevice();
    Device->SetName("Main Vulkan Device");

    ShaderCompiler = std::make_unique<FVulkanShaderCompiler>();
    ShaderCompiler->SetOptimizationLevel(FVulkanShaderCompiler::EOptimizationLevel::PerfWithDebug);
}

void FVulkanDynamicRHI::PostInit()
{
    IMGUI_CHECKVERSION();
}

void FVulkanDynamicRHI::FlushDeletionQueue()
{
    int Counter = 0;
    for (std::function<void()>& DeletionFunction: DeletionQueue) {
        DeletionFunction();
        Counter++;
    }
    DeletionQueue.Clear();
    if (Counter > 0) {
        LOG(LogVulkanRHI, Info, "Deleted {} RHI ressources", Counter);
    }
}

void FVulkanDynamicRHI::DeferedDeletion(std::function<void()>&& InDeletionFunction)
{
    DeletionQueue.Emplace(std::move(InDeletionFunction));
}

void FVulkanDynamicRHI::RegisterScene(WeakRef<RHIScene> Scene)
{
    ScenesContainers.Emplace(Scene);
}

void FVulkanDynamicRHI::Shutdown()
{
    WaitUntilIdle();

    ShaderCompiler.reset();

    /// Release the command contexts
    RHIReleaseCommandContext(Device->GetImmediateContext());
    AvailableCommandContexts.Clear(true);
    check(CommandContexts.IsEmpty());

    FlushDeletionQueue();    // Flush the deletion queue

    Device.reset();

#if VULKAN_DEBUGGING_ENABLED
    DebugLayer.RemoveDebugLayer(m_Instance);
#endif

    VulkanAPI::vkDestroyInstance(m_Instance, VULKAN_CPU_ALLOCATOR);
    m_Instance = VK_NULL_HANDLE;
}

VkInstance FVulkanDynamicRHI::CreateInstance(const TArray<const char*>& ValidationLayers)
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

    FVulkanInstanceExtensionArray Extensions = FVulkanPlatform::GetInstanceExtensions();
    TArray<const char*> InstanceExtensions;
    for (const std::unique_ptr<IInstanceVulkanExtension>& Extension: Extensions) {
        Extension->PreInstanceCreated(InstInfo);
        InstanceExtensions.AddUnique(Extension->GetExtensionName());
    }
    InstInfo.enabledExtensionCount = InstanceExtensions.Size();
    InstInfo.ppEnabledExtensionNames = InstanceExtensions.Raw();

#if VULKAN_DEBUGGING_ENABLED
    InstInfo.enabledLayerCount = ValidationLayers.Size();
    InstInfo.ppEnabledLayerNames = ValidationLayers.Raw();
#endif

    VkInstance Instance = VK_NULL_HANDLE;
    VkResult Result = VulkanAPI::vkCreateInstance(&InstInfo, VULKAN_CPU_ALLOCATOR, &Instance);

    if (Result == VK_ERROR_INCOMPATIBLE_DRIVER) {
        FPlatformMisc::DisplayMessageBox(
            EBoxMessageType::Ok, "Unable to initialize Vulkan.",
            "Unable to load Vulkan library and/or acquire the necessary function pointers. Make sure an "
            "up-to-date libvulkan.so.1 is installed.");
        LOG(LogVulkanRHI, Fatal, "Cannot find a compatible Vulkan driver.");
        Utils::RequestExit(1);
    } else if (Result == VK_ERROR_EXTENSION_NOT_PRESENT) {
        std::string MissingExtensions = GetMissingExtensions(InstanceExtensions);

        FPlatformMisc::DisplayMessageBox(
            EBoxMessageType::Ok, "Incompatible Vulkan driver found!",
            std::format("Vulkan driver does not contain specified extensions:\n{:s}\nMake sure your layers "
                        "path is set appropriately.",
                        MissingExtensions));
        LOG(LogVulkanRHI, Fatal, "Extension not found : {} !", MissingExtensions);
        Utils::RequestExit(1);
    } else if (Result != VK_SUCCESS) {
        LOG(LogVulkanRHI, Fatal, "Vulkan failed to create instance! {:s}", magic_enum::enum_name(Result));
        FPlatformMisc::DisplayMessageBox(EBoxMessageType::Ok, "No Vulkan driver found!",
                                         "Vulkan failed to create instance !\n\nDo you have a compatible Vulkan "
                                         "driver (ICD) installed?");
        Utils::RequestExit(1);
    }

    VK_CHECK_RESULT(Result);

    if (!FVulkanPlatform::LoadVulkanInstanceFunctions(Instance)) {
        LOG(LogVulkanRHI, Fatal, "Couldn't find some of Vulkan's entry points !");
        FPlatformMisc::DisplayMessageBox(EBoxMessageType::Ok,
                                         "Failed to find all required Vulkan entry points! Try updating your driver.",
                                         "No Vulkan entry points found!");
        Utils::RequestExit(1);
    }

    LOG(LogVulkanRHI, Info, "Using {} Instance extensions {}", InstanceExtensions.Size(),
        InstanceExtensions.Size() ? ":" : ".");
    for (const char* Layer: InstanceExtensions) {
        LOG(LogVulkanRHI, Info, "* {}", Layer);
    }

    return Instance;
}

FVulkanDevice* FVulkanDynamicRHI::SelectDevice(VkInstance Instance)
{
    std::uint32_t GpuCount = 0;
    VkResult Result = VulkanAPI::vkEnumeratePhysicalDevices(Instance, &GpuCount, nullptr);
    if (Result == VK_ERROR_INITIALIZATION_FAILED) {
        LOG(LogVulkanRHI, Fatal, "Vulkan failed to find enumerate device!");
        return nullptr;
    }
    VK_CHECK_RESULT_EXPANDED(Result);
    checkMsg(GpuCount >= 1, "No GPU(s)/Driver(s) that support Vulkan were found!");

    TArray<VkPhysicalDevice> PhysicalDevices(GpuCount);
    VK_CHECK_RESULT_EXPANDED(VulkanAPI::vkEnumeratePhysicalDevices(Instance, &GpuCount, PhysicalDevices.Raw()));
    checkMsg(GpuCount >= 1, "Couldn't enumerate physical devices!");

    struct FDeviceInfo {
        FVulkanDevice* Device;
        std::uint32_t DeviceIndex;
    };
    TArray<FVulkanDevice*> Devices;
    TArray<FDeviceInfo> DiscreteDevice;
    TArray<FDeviceInfo> IntegratedDevice;

    // Sort the physical devices into discrete and integrated
    LOG(LogVulkanRHI, Info, "Found {} device(s)", GpuCount);
    for (std::uint32_t Index = 0; Index < GpuCount; Index++) {
        LOG(LogVulkanRHI, Info, "Device {}:", Index);
        FVulkanDevice* NewDevice = new FVulkanDevice(PhysicalDevices[Index]);
        Devices.Add(NewDevice);

        const bool bIsDiscrete = (NewDevice->GetDeviceProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
        const bool bIsCPUDevice = (NewDevice->GetDeviceProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU);

        if (bIsDiscrete) {
            DiscreteDevice.Emplace(NewDevice, Index);
        } else if (bIsCPUDevice) {
            LOG(LogVulkanRHI, Info, "Skipping device[{}] of type VK_PHYSICAL_DEVICE_TYPE_CPU",
                NewDevice->GetDeviceProperties().deviceName);
        } else {
            IntegratedDevice.Emplace(NewDevice, Index);
        }
    }

    // merge the two arrays, so that if DiscreteDevice is empty, we can use IntegratedDevice
    DiscreteDevice.Append(IntegratedDevice);

    uint32 DeviceIndex = (uint32)-1;
    FVulkanDevice* SelectedDevice = nullptr;
    if (DiscreteDevice.Size() > 0) {
        SelectedDevice = DiscreteDevice[0].Device;
        DeviceIndex = DiscreteDevice[0].DeviceIndex;
    } else if (IntegratedDevice.Size() > 0) {
        SelectedDevice = IntegratedDevice[0].Device;
        DeviceIndex = IntegratedDevice[0].DeviceIndex;
    }

    // Remove all the other devices
    for (FVulkanDevice* const Device: Devices) {
        if (Device != SelectedDevice) {
            delete Device;
        }
    }

    if (SelectedDevice == nullptr) {
        LOG(LogVulkanRHI, Info, "Cannot find compatible Vulkan device");
        return nullptr;
    }
    LOG(LogVulkanRHI, Info, "Chosen device index: {}", DeviceIndex);
    return SelectedDevice;
}

}    // namespace VulkanRHI

static std::string GetMissingExtensions(TArray<const char*> VulkanExtensions)
{
    std::string MissingExtensions;
    uint32_t PropertyCount;
    VulkanRHI::VulkanAPI::vkEnumerateInstanceExtensionProperties(nullptr, &PropertyCount, nullptr);

    TArray<VkExtensionProperties> Properties;
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

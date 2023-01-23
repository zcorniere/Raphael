#include "Engine/Renderer/Vulkan/VulkanDynamicRHI.hxx"

#include "Engine/Renderer/Vulkan/VulkanDevice.hxx"
#include "Engine/Renderer/Vulkan/VulkanGenericPlatform.hxx"
#include "Engine/Renderer/Vulkan/VulkanLoader.hxx"
#include "Engine/Renderer/Vulkan/VulkanUtils.hxx"

namespace Raphael::RHI
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
}

void VulkanDynamicRHI::PostInit()
{
}

void VulkanDynamicRHI::Shutdown()
{
    Device->Destroy();

    delete Device;
    Device = nullptr;

#if VULKAN_DEBUGGING_ENABLED
    RemoveDebugLayerCallback();
#endif

    VulkanAPI::vkDestroyInstance(m_Instance, nullptr);
    VulkanPlatform::FreeVulkanLibrary();
}

void VulkanDynamicRHI::CreateInstance()
{
    VkApplicationInfo AppInfo;
    ZeroVulkanStruct(AppInfo, VK_STRUCTURE_TYPE_APPLICATION_INFO);
    AppInfo.pEngineName = "RaphaelEngine";
    AppInfo.apiVersion = VK_API_VERSION_1_2;

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
}

}    // namespace Raphael::RHI

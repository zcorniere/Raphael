#include "Engine/Core/Window.hxx"
#include "GLFW/glfw3.h"

#include "VulkanRHI/VulkanExtension.hxx"
#include "VulkanRHI/VulkanPlatform.hxx"

template <typename ExistingChainType, typename NewStructType>
static void AddToPNext(ExistingChainType& Existing, NewStructType& Added)
{
    Added.pNext = (void*)Existing.pNext;
    Existing.pNext = (void*)&Added;
}

namespace VulkanRHI
{

class DynamicRenderingExtension : public IDeviceVulkanExtension
{
public:
    DynamicRenderingExtension(): IDeviceVulkanExtension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
    {
        std::memset(&DynamicRenderingFeature, 0, sizeof(DynamicRenderingFeature));
        DynamicRenderingFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        DynamicRenderingFeature.dynamicRendering = VK_TRUE;
    }

    void PreDeviceCreated(VkDeviceCreateInfo& DeviceInfo) override final
    {
        AddToPNext(DeviceInfo, DynamicRenderingFeature);
    }

private:
    VkPhysicalDeviceDynamicRenderingFeatures DynamicRenderingFeature{};
};

#define ADD_SIMPLE_EXTENSION(Array, ExtensionType, ExtensionName) \
    Array.AddUnique(std::make_unique<ExtensionType>(ExtensionName))
#define ADD_COMPLEX_ENTENSION(Array, ExtensionType) Array.AddUnique(std::make_unique<ExtensionType>())

VulkanInstanceExtensionArray VulkanPlatform::GetInstanceExtensions()
{

    VulkanInstanceExtensionArray InstanceExtension;
    Window::EnsureGLFWInit();

    uint32 glfwExtensionCount = 0;
    const char** glfwExtentsions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    for (uint32 i = 0; i < glfwExtensionCount; i++) {
        ADD_SIMPLE_EXTENSION(InstanceExtension, IInstanceVulkanExtension, glfwExtentsions[i]);
    }

    ADD_SIMPLE_EXTENSION(InstanceExtension, IInstanceVulkanExtension, VK_KHR_SURFACE_EXTENSION_NAME);
#if VULKAN_DEBUGGING_ENABLED
    ADD_SIMPLE_EXTENSION(InstanceExtension, IInstanceVulkanExtension, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif    // VULKAN_DEBUGGING_ENABLED
    return InstanceExtension;
}

VulkanDeviceExtensionArray VulkanPlatform::GetDeviceExtensions()
{
    VulkanDeviceExtensionArray DeviceExtension;

    ADD_SIMPLE_EXTENSION(DeviceExtension, IDeviceVulkanExtension, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    ADD_SIMPLE_EXTENSION(DeviceExtension, IDeviceVulkanExtension, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);

    ADD_COMPLEX_ENTENSION(DeviceExtension, DynamicRenderingExtension);

    return DeviceExtension;
}

#undef ADD_SIMPLE_EXTENSION
#undef ADD_COMPLEX_ENTENSION

}    // namespace VulkanRHI

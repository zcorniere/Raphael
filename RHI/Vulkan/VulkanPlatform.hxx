#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

namespace VulkanRHI
{

class VulkanDevice;

class VulkanPlatform
{
public:
    static bool LoadVulkanLibrary();
    static bool LoadVulkanInstanceFunctions(VkInstance inInstance);
    static void FreeVulkanLibrary();

    static void GetInstanceExtensions(std::vector<const char *> &OutExtensions);
    static void GetInstanceLayers(std::vector<const char *> &OutLayers);
    
    static void GetDeviceExtensions(Ref<VulkanDevice> Device, std::vector<const char *> &OutExtensions);
    static void GetDeviceLayers(std::vector<const char *> &OutLayers);

    static void CreateSurface(void *WindowHandle, VkInstance Instance, VkSurfaceKHR *OutSurface);

    static VkResult Present(VkQueue Queue, VkPresentInfoKHR &PresentInfo);

    static VkResult CreateSwapchainKHR(VkDevice Device, const VkSwapchainCreateInfoKHR *CreateInfo,
                                       const VkAllocationCallbacks *Allocator, VkSwapchainKHR *Swapchain);
    static void DestroySwapchainKHR(VkDevice Device, VkSwapchainKHR Swapchain, const VkAllocationCallbacks *Allocator);
};

}    // namespace VulkanRHI



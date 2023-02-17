#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

class VulkanDevice;

class VulkanGenericPlatform : public RObject
{
public:
    static bool LoadVulkanLibrary()
    {
        return true;
    }
    static bool LoadvulkanInstanceFunction([[maybe_unused]] VkInstance inInstance)
    {
        return true;
    }
    static void FreeVulkanLibrary()
    {
    }

    static void GetInstanceExtensions(std::vector<const char *> &OutExtensions);
    static void GetInstanceLayers([[maybe_unused]] std::vector<const char *> &OutLayers)
    {
    }
    static void GetDeviceExtensions(VulkanDevice *Device, std::vector<const char *> &OutExtensions);
    static void GetDeviceLayers([[maybe_unused]] std::vector<const char *> &OutLayers)
    {
    }

    static void CreateSurface(VkSurfaceKHR *OutSurface);

    static VkResult Present(VkQueue Queue, VkPresentInfoKHR &PresentInfo);

    static VkResult CreateSwapchainKHR(VkDevice Device, const VkSwapchainCreateInfoKHR *CreateInfo,
                                       const VkAllocationCallbacks *Allocator, VkSwapchainKHR *Swapchain);
    static void DestroySwapchainKHR(VkDevice Device, VkSwapchainKHR Swapchain, const VkAllocationCallbacks *Allocator);
};

#if defined(PLATFORM_WINDOWS)
    #error "Unsupported Platform"
#elif defined(PLATFORM_LINUX)
    #include "Engine/Renderer/Vulkan/Linux/VulkanLinuxPlatform.hxx"
#else
    #error "Unsupported Platform"
#endif

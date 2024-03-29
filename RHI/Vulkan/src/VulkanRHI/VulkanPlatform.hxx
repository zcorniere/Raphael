#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

class Window;

namespace VulkanRHI
{

class VulkanDevice;

class VulkanPlatform
{
public:
    static bool LoadVulkanLibrary();
    static bool LoadVulkanInstanceFunctions(VkInstance inInstance);
    static void FreeVulkanLibrary();

    static void GetInstanceExtensions(Array<const char*>& OutExtensions);
    static void GetInstanceLayers(Array<const char*>& OutLayers);

    static void GetDeviceExtensions(VulkanDevice* Device, Array<const char*>& OutExtensions);
    static void GetDeviceLayers(Array<const char*>& OutLayers);

    static void CreateSurface(Window* WindowHandle, VkInstance Instance, VkSurfaceKHR* OutSurface);
};

}    // namespace VulkanRHI

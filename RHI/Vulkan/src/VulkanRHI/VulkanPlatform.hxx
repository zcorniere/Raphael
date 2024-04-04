#pragma once

#include "VulkanRHI/VulkanExtension.hxx"

class Window;

namespace VulkanRHI
{

class VulkanDevice;

using VulkanDeviceExtensionArray = Array<std::unique_ptr<IDeviceVulkanExtension>>;
using VulkanInstanceExtensionArray = Array<std::unique_ptr<IInstanceVulkanExtension>>;

class VulkanPlatform
{
public:
    static bool LoadVulkanLibrary();
    static bool LoadVulkanInstanceFunctions(VkInstance inInstance);
    static void FreeVulkanLibrary();

    static VulkanInstanceExtensionArray GetInstanceExtensions();
    static void GetInstanceLayers(Array<const char*>& OutLayers);

    static VulkanDeviceExtensionArray GetDeviceExtensions();
    static void GetDeviceLayers(Array<const char*>& OutLayers);

    static void CreateSurface(Window* WindowHandle, VkInstance Instance, VkSurfaceKHR* OutSurface);
};

}    // namespace VulkanRHI

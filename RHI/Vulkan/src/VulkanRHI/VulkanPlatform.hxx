#pragma once

#include "VulkanRHI/VulkanExtension.hxx"

class RWindow;

namespace VulkanRHI
{

class FVulkanDevice;

using FVulkanDeviceExtensionArray = TArray<std::unique_ptr<IDeviceVulkanExtension>>;
using FVulkanInstanceExtensionArray = TArray<std::unique_ptr<IInstanceVulkanExtension>>;

class FVulkanPlatform
{
public:
    static bool LoadVulkanLibrary();
    static bool LoadVulkanInstanceFunctions(VkInstance inInstance);
    static void FreeVulkanLibrary();

    static FVulkanInstanceExtensionArray GetInstanceExtensions();
    static void GetInstanceLayers(TArray<const char*>& OutLayers);

    static FVulkanDeviceExtensionArray GetDeviceExtensions();
    static void GetDeviceLayers(TArray<const char*>& OutLayers);

    static void CreateSurface(RWindow* WindowHandle, VkInstance Instance, VkSurfaceKHR* OutSurface);
};

}    // namespace VulkanRHI

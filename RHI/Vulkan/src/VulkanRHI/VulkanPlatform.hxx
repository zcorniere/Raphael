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
    static TArray<VkExtensionProperties> GetDriverSupportedInstanceExtensions(const char* LayerName);
    static void GetInstanceLayers(TArray<const char*>& OutLayers);

    static FVulkanDeviceExtensionArray GetDeviceExtensions();
    static TArray<VkExtensionProperties> GetDriverSupportedDeviceExtensions(VkPhysicalDevice Gpu,
                                                                            const char* LayerName);
    static void GetDeviceLayers(TArray<const char*>& OutLayers);

    static void CreateSurface(RWindow* WindowHandle, VkInstance Instance, VkSurfaceKHR* OutSurface);
};

}    // namespace VulkanRHI

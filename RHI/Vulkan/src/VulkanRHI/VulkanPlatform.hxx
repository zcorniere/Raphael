#pragma once

#include "VulkanRHI/VulkanExtension.hxx"

class RWindow;
class IExternalModule;

namespace VulkanRHI
{

class FVulkanDevice;

using FVulkanDeviceExtensionArray = TArray<std::unique_ptr<IDeviceVulkanExtension>>;
using FVulkanInstanceExtensionArray = TArray<std::unique_ptr<IInstanceVulkanExtension>>;

class FVulkanPlatform
{
public:
    bool LoadVulkanLibrary();
    bool LoadVulkanInstanceFunctions(VkInstance inInstance);
    void FreeVulkanLibrary();

    FVulkanInstanceExtensionArray GetInstanceExtensions() const;
    TArray<VkExtensionProperties> GetDriverSupportedInstanceExtensions(const char* LayerName) const;
    void GetInstanceLayers(TArray<const char*>& OutLayers) const;

    FVulkanDeviceExtensionArray GetDeviceExtensions() const;
    TArray<VkExtensionProperties> GetDriverSupportedDeviceExtensions(VkPhysicalDevice Gpu, const char* LayerName) const;
    void GetDeviceLayers(TArray<const char*>& OutLayers) const;

    void CreateSurface(RWindow* WindowHandle, VkInstance Instance, VkSurfaceKHR* OutSurface);

private:
private:
    Ref<IExternalModule> VulkanModuleHandle = nullptr;
};

}    // namespace VulkanRHI

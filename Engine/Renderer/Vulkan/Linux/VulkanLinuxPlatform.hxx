#pragma once

#include "Engine/Renderer/Vulkan/VulkanGenericPlatform.hxx"

// No Specific Entry point for Linux Vulkan
#define VK_ENTRYPOINTS_PLATFORM_BASE(LoadMacro)

// No Specific Entry point for Linux Vulkan
#define VK_ENTRYPOINTS_PLATFORM_INSTANCE(LoadMacro)

class VulkanLinuxPlatform : public VulkanGenericPlatform
{
public:
    static bool LoadVulkanLibrary();
    static bool LoadVulkanInstanceFunctions(VkInstance inInstance);
    static void FreeVulkanLibrary();

    static void GetInstanceExtensions(std::vector<const char *> &OutExtensions);
    static void GetInstanceLayers([[maybe_unused]] std::vector<const char *> &OutLayers)
    {
    }
    static void GetDeviceExtensions(VulkanDevice *Device, std::vector<const char *> &OutExtensions);
    static void GetDeviceLayers([[maybe_unused]] std::vector<const char *> &OutLayers)
    {
    }

    static void CreateSurface(void *WindowHandle, VkInstance Instance, VkSurfaceKHR *OutSurface);

protected:
    static void *VulkanLib;
    static bool bAttemptedLoad;
};

using VulkanPlatform = VulkanLinuxPlatform;

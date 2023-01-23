#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include "Engine/Renderer/Vulkan/VulkanDynamicRHI.hxx"

namespace Raphael::RHI
{

class VulkanDevice
{
public:
    VulkanDevice(VulkanDynamicRHI *InRHI, VkPhysicalDevice Gpu);
    ~VulkanDevice();

    void InitGPU();
    void CreateDevice(std::vector<const char *> DeviceLayers, std::vector<const char *> DeviceExtensions);

    void PrepareForDestroy();
    void Destroy();

    void WaitUntilIdle();

    VkPhysicalDevice GetPhysicalHandle() const
    {
        return Gpu;
    }
    inline VkDevice GetInstanceHandle() const
    {
        return Device;
    }

private:
    VkDevice Device;
    VkPhysicalDevice Gpu;
    VkPhysicalDeviceProperties GpuProps;

    VkPhysicalDeviceFeatures PhysicalFeatures;
};

}    // namespace Raphael::RHI

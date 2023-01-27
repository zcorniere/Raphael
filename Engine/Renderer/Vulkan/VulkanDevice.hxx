#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include "Engine/Renderer/Vulkan/VulkanDynamicRHI.hxx"
#include "Engine/Renderer/Vulkan/VulkanQueue.hxx"

namespace Raphael::RHI
{

class VulkanDevice : public RObject
{
public:
    VulkanDevice(Ref<VulkanDynamicRHI> InRHI, VkPhysicalDevice Gpu);
    ~VulkanDevice();

    void InitGPU();
    void CreateDevice(const std::vector<const char *> &DeviceLayers, const std::vector<const char *> &DeviceExtensions);

    void PrepareForDestroy();
    void Destroy();

    void WaitUntilIdle();

    inline VkPhysicalDevice GetPhysicalHandle() const
    {
        return Gpu;
    }
    inline VkDevice GetInstanceHandle() const
    {
        return Device;
    }
    inline const VkPhysicalDeviceProperties &GetDeviceProperties() const
    {
        return GpuProps;
    }

private:
    Ref<VulkanDynamicRHI> RHI;

    VkDevice Device;
    VkPhysicalDevice Gpu;
    VkPhysicalDeviceProperties GpuProps;

    VkPhysicalDeviceFeatures PhysicalFeatures;
    std::vector<VkQueueFamilyProperties> QueueFamilyProps;

    Ref<VulkanQueue> GraphicsQueue;
    Ref<VulkanQueue> ComputeQueue;
    Ref<VulkanQueue> TransferQueue;
    Ref<VulkanQueue> PresentQueue;
};

}    // namespace Raphael::RHI

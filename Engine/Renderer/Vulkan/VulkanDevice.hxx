#pragma once

#include "Engine/Core/RObject.hxx"
#include "Engine/Renderer/Vulkan/Vulkan.hxx"
#include "Engine/Renderer/Vulkan/VulkanPhysicalDevice.hxx"

#include <map>
#include <unordered_set>

namespace Raphael
{

// Represents a logical device
class VulkanDevice : public RObject
{
public:
    VulkanDevice(const Ref<VulkanPhysicalDevice> &physicalDevice, VkPhysicalDeviceFeatures enabledFeatures);
    ~VulkanDevice();

    void Destroy();

    VkQueue GetGraphicsQueue()
    {
        return m_GraphicsQueue;
    }
    VkQueue GetComputeQueue()
    {
        return m_ComputeQueue;
    }

    VkCommandBuffer GetCommandBuffer(bool begin, bool compute = false);
    void FlushCommandBuffer(VkCommandBuffer commandBuffer);
    void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue);

    VkCommandBuffer CreateSecondaryCommandBuffer(const char *debugName);

    const Ref<VulkanPhysicalDevice> &GetPhysicalDevice() const
    {
        return m_PhysicalDevice;
    }
    VkDevice GetVulkanDevice() const
    {
        return m_LogicalDevice;
    }

private:
    VkDevice m_LogicalDevice = nullptr;
    Ref<VulkanPhysicalDevice> m_PhysicalDevice;
    VkPhysicalDeviceFeatures m_EnabledFeatures;

    VkQueue m_GraphicsQueue;
    VkQueue m_ComputeQueue;

    bool m_EnableDebugMarkers = false;
};

}    // namespace Raphael

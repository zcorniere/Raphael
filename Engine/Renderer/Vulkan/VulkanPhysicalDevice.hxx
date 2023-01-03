#pragma once

#include "Engine/Core/RObject.hxx"
#include "Engine/Renderer/Vulkan/Vulkan.hxx"

#include <unordered_set>

namespace Raphael
{

class VulkanPhysicalDevice : public RObject
{
public:
    struct QueueFamilyIndices {
        std::int32_t Graphics = -1;
        std::int32_t Compute = -1;
        std::int32_t Transfer = -1;
    };

public:
    VulkanPhysicalDevice();
    ~VulkanPhysicalDevice();

    bool IsExtensionSupported(const std::string &extensionName) const;
    std::uint32_t GetMemoryTypeIndex(std::uint32_t typeBits, VkMemoryPropertyFlags properties) const;

    VkPhysicalDevice GetVulkanPhysicalDevice() const
    {
        return m_PhysicalDevice;
    }
    const QueueFamilyIndices &GetQueueFamilyIndices() const
    {
        return m_QueueFamilyIndices;
    }

    const VkPhysicalDeviceProperties &GetProperties() const
    {
        return m_Properties;
    }
    const VkPhysicalDeviceLimits &GetLimits() const
    {
        return m_Properties.limits;
    }
    const VkPhysicalDeviceMemoryProperties &GetMemoryProperties() const
    {
        return m_MemoryProperties;
    }

    VkFormat GetDepthFormat() const
    {
        return m_DepthFormat;
    }

    static Ref<VulkanPhysicalDevice> Select();

private:
    VkFormat FindDepthFormat() const;
    QueueFamilyIndices GetQueueFamilyIndices(int queueFlags);

private:
    QueueFamilyIndices m_QueueFamilyIndices;

    VkPhysicalDevice m_PhysicalDevice = nullptr;
    VkPhysicalDeviceProperties m_Properties;
    VkPhysicalDeviceFeatures m_Features;
    VkPhysicalDeviceMemoryProperties m_MemoryProperties;

    VkFormat m_DepthFormat = VK_FORMAT_UNDEFINED;

    std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties;
    std::unordered_set<std::string> m_SupportedExtensions;
    std::vector<VkDeviceQueueCreateInfo> m_QueueCreateInfos;

    friend class VulkanDevice;
};

}    // namespace Raphael

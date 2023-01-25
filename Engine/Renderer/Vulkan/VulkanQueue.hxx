#pragma once

#include "VulkanUtils.hxx"

namespace Raphael::RHI
{

class VulkanDevice;

class VulkanQueue
{
public:
    VulkanQueue(VulkanDevice *InDevice, std::uint32_t InFamilyIndex);
    ~VulkanQueue();

    inline std::uint32_t GetFamilyIndex() const
    {
        return FamilyIndex;
    }

    inline std::uint32_t GetQueueIndex() const
    {
        return QueueIndex;
    }

    inline VkQueue GetHandle() const
    {
        return Queue;
    }

private:
    VkQueue Queue;
    std::uint32_t FamilyIndex;
    std::uint32_t QueueIndex;
    VulkanDevice *Device;
};

}    // namespace Raphael::RHI

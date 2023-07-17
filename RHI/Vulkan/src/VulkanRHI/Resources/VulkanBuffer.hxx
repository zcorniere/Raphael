#pragma once

#include "Engine/Core/RHI/Resources/RHIBuffer.hxx"

#include "VulkanRHI/VulkanLoader.hxx"


namespace VulkanRHI
{

class VulkanDevice;
class VulkanMemoryAllocation;

class VulkanBuffer : public RHIBuffer
{
public:
    VulkanBuffer(Ref<VulkanDevice>& InDevice, const uint32 InSize, const EBufferUsageFlags InUsage, const uint32 InStride,
                 Ref<ResourceArray> &InitialData);
    ~VulkanBuffer();

    void SetName(std::string_view InName) override;

    inline VkBuffer GetHandle() const
    {
        return BufferHandle;
    }

    /// Offset used for Binding a VkBuffer
    inline uint32 GetOffset() const
    {
        return Offset;
    }

    /// Remaining size from the current offset
    uint64 GetCurrentSize() const;
    inline VkBufferUsageFlags GetBufferUsageFlags() const
    {
        return BufferUsageFlags;
    }

    inline VkIndexType GetIndexType() const
    {
        return GetStride() == 4 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;
    }

private:
    Ref<VulkanDevice> Device;

    VkMemoryRequirements MemoryRequirements;
    VkBuffer BufferHandle;
    Ref<VulkanMemoryAllocation> Memory;

    uint32 Offset;
    VkBufferUsageFlags BufferUsageFlags;
};

}    // namespace VulkanRHI

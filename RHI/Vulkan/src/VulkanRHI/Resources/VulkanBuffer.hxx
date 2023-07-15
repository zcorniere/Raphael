#pragma once

#include "Engine/Core/RHI/Resources/RHIBuffer.hxx"

#include "VulkanRHI/VulkanLoader.hxx"

namespace VulkanRHI
{

class VulkanDevice;
class VulkanMemoryAllocation;

class VulkanBuffer final : public RHIBuffer
{
public:
    VulkanBuffer(VulkanDevice* InDevice, const uint32 InSize, const EBufferUsageFlags InUsage, const uint32 InStride,
                 ResourceArray* InitialData);
    ~VulkanBuffer();

    virtual void SetName(std::string_view InName) override;
    virtual std::string_view GetTypeName_Internal() const override
    {
        return type_name<VulkanBuffer>();
    }

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
    VulkanDevice* Device;

    VkMemoryRequirements MemoryRequirements;
    VkBuffer BufferHandle;
    VulkanMemoryAllocation* Memory;

    uint32 Offset;
    uint32 Stride;
    VkBufferUsageFlags BufferUsageFlags;
};

}    // namespace VulkanRHI

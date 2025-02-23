#pragma once

#include "Engine/Core/RHI/Resources/RHIBuffer.hxx"

namespace VulkanRHI
{

class FVulkanDevice;
class RVulkanMemoryAllocation;

class RVulkanBuffer : public RRHIBuffer, public IDeviceChild
{
    RTTI_DECLARE_TYPEINFO(RVulkanBuffer, RRHIBuffer);

public:
    RVulkanBuffer(FVulkanDevice* InDevice, const FRHIBufferDesc& InDescription);
    virtual ~RVulkanBuffer();

    void SetName(std::string_view InName) override;

    inline VkBuffer GetHandle() const
    {
        return BufferHandle;
    }
    inline RVulkanMemoryAllocation* GetMemory()
    {
        return Memory.Raw();
    }

    /// Remaining size from the current offset
    inline uint32 GetCurrentSize() const
    {
        return Description.Size;
    }

    inline VkIndexType GetIndexType() const
    {
        return GetStride() == 4 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;
    }

    VkDescriptorBufferInfo GetDescriptorBufferInfo() const
    {
        return VkDescriptorBufferInfo{
            .buffer = BufferHandle,
            .offset = 0,
            .range = Description.Size,
        };
    }

private:
    VkBuffer BufferHandle;
    Ref<RVulkanMemoryAllocation> Memory;
};

}    // namespace VulkanRHI

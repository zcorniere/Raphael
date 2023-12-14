#pragma once

#include "Engine/Core/RHI/Resources/RHIBuffer.hxx"

namespace VulkanRHI
{

class VulkanDevice;
class VulkanMemoryAllocation;

class VulkanBuffer : public RHIBuffer, public IDeviceChild
{
public:
    VulkanBuffer(VulkanDevice* InDevice, const RHIBufferDesc& InDescription);
    ~VulkanBuffer();

    void SetName(std::string_view InName) override;

    inline VkBuffer GetHandle() const
    {
        return BufferHandle;
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

private:
    VkMemoryRequirements MemoryRequirements;
    VkBuffer BufferHandle;
    Ref<VulkanMemoryAllocation> Memory;
};

}    // namespace VulkanRHI

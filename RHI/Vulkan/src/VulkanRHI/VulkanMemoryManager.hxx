#pragma once

#include <vk_mem_alloc.h>

namespace VulkanRHI
{

class FVulkanDevice;

class FVulkanMemoryManager;

class RVulkanMemoryAllocation : public RObject
{
    RTTI_DECLARE_TYPEINFO(RVulkanMemoryAllocation, RObject);

public:
    RVulkanMemoryAllocation() = delete;
    explicit RVulkanMemoryAllocation(FVulkanMemoryManager& InManager);

    void SetName(std::string_view InName) override;

    void* Map(VkDeviceSize InSize, VkDeviceSize Offset = 0);
    void Unmap();

    bool CanBeMapped() const
    {
        return bCanBeMapped != 0;
    }
    bool IsMapped() const
    {
        return AllocationInfo.pMappedData || MappedPointer;
    }
    void* GetMappedPointer()
    {
        check(IsMapped());
        if (AllocationInfo.pMappedData) {
            return AllocationInfo.pMappedData;
        }
        return MappedPointer;
    }
    bool IsCoherent() const
    {
        return bIsCoherent;
    }

    void FlushMappedMemory(VkDeviceSize InOffset, VkDeviceSize InSize);
    void InvalidateMappedMemory(VkDeviceSize InOffset, VkDeviceSize InSize);
    void BindBuffer(VkBuffer Buffer);
    void BindImage(VkImage Image);

    VmaAllocation& GetHandle()
    {
        return Allocation;
    }
    VkDeviceSize GetSize() const
    {
        return Size;
    }

private:
    FVulkanMemoryManager& ManagerHandle;

    VmaAllocation Allocation = VK_NULL_HANDLE;
    VmaAllocationInfo AllocationInfo;

    VkDeviceSize Size = 0;
    void* MappedPointer = nullptr;
    bool bCanBeMapped = false;
    bool bIsCoherent = false;

    friend class FVulkanMemoryManager;
};

class FVulkanMemoryManager : public IDeviceChild
{
public:
    explicit FVulkanMemoryManager(FVulkanDevice* InDevice);
    ~FVulkanMemoryManager();

    [[nodiscard]] Ref<RVulkanMemoryAllocation> Alloc(const VkMemoryRequirements& MemoryRequirement,
                                                     VmaMemoryUsage MemUsage, bool Mappable);
    [[nodiscard]] std::pair<VkBuffer, Ref<RVulkanMemoryAllocation>>
    Alloc(const VkBufferCreateInfo& BufferCreateInfo, const VmaAllocationCreateInfo& AllocCreateInfo);

    void Free(Ref<RVulkanMemoryAllocation>& Allocation);

    uint64 GetTotalMemory(bool bGPUOnly) const;
    void PrintMemInfo() const;

    VmaAllocator GetAllocator() const
    {
        return Allocator;
    }

private:
    VmaAllocationCreateInfo GetCreateInfo(VmaMemoryUsage MemUsage, bool Mappable);

private:
    VmaAllocator Allocator;

    VkPhysicalDeviceMemoryProperties MemoryProperties;
    std::atomic<uint32> AllocationCount;

#if VULKAN_DEBUGGING_ENABLED
    TArray<WeakRef<RVulkanMemoryAllocation>> MemoryAllocationArray;
    std::mutex MemoryAllocationArrayMutex;
#endif    // VULKAN_DEBUGGING_ENABLED

    friend RVulkanMemoryAllocation;
};

}    // namespace VulkanRHI

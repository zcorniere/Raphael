#pragma once

#include <vk_mem_alloc.h>

namespace VulkanRHI
{

class VulkanDevice;

class VulkanMemoryManager;

class VulkanMemoryAllocation : public RObject
{
public:
    VulkanMemoryAllocation() = delete;
    explicit VulkanMemoryAllocation(VulkanMemoryManager& InManager);

    void SetName(std::string_view InName) override;

    void* Map(VkDeviceSize InSize, VkDeviceSize Offset = 0);
    void Unmap();

    bool CanBeMapped() const
    {
        return bCanBeMapped != 0;
    }
    bool IsMapped() const
    {
        return MappedPointer;
    }
    void* GetMappedPointer()
    {
        check(IsMapped());
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
    VmaAllocation Allocation;
    VmaAllocationInfo AllocationInfo;

    VkDeviceSize Size;
    void* MappedPointer = nullptr;
    VulkanMemoryManager& ManagerHandle;
    bool bCanBeMapped;
    bool bIsCoherent;

    friend class VulkanMemoryManager;
};

class VulkanMemoryManager : public IDeviceChild
{
public:
    explicit VulkanMemoryManager(VulkanDevice* InDevice);
    ~VulkanMemoryManager();

    Ref<VulkanMemoryAllocation> Alloc(const VkMemoryRequirements& MemoryRequirement, VmaMemoryUsage MemUsage,
                                      bool Mappable);
    void Free(Ref<VulkanMemoryAllocation>& Allocation);

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
    Array<WeakRef<VulkanMemoryAllocation>> MemoryAllocationArray;
    std::mutex MemoryAllocationArrayMutex;
#endif    // VULKAN_DEBUGGING_ENABLED

    friend VulkanMemoryAllocation;
};

}    // namespace VulkanRHI

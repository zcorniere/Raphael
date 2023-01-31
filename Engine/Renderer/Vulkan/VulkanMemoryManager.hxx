#pragma once

#include <vk_mem_alloc.h>

namespace Raphael::RHI
{

class VulkanDevice;

class VulkanMemoryManager;

class VulkanMemoryAllocation : public RObject
{
public:
    VulkanMemoryAllocation() = delete;
    VulkanMemoryAllocation(Ref<VulkanMemoryManager> InManager)
        : Size(0), MappedPointer(nullptr), ManagerHandle(InManager), bCanBeMapped(false), bIsCoherent(false)
    {
    }

    void *Map(VkDeviceSize InSize, VkDeviceSize Offset = 0);
    void Unmap();

    bool CanBeMapped() const
    {
        return bCanBeMapped != 0;
    }
    bool IsMapped() const
    {
        return MappedPointer;
    }
    void *GetMappedPointer()
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

    VmaAllocation &GetHandle()
    {
        return Allocation;
    }
    VkDeviceSize GetSize() const
    {
        return Size;
    }

private:
    VmaAllocation Allocation;

    VkDeviceSize Size;
    void *MappedPointer = nullptr;
    Ref<VulkanMemoryManager> ManagerHandle;
    bool bCanBeMapped;
    bool bIsCoherent;
};

class VulkanMemoryManager : public RObject
{
public:
    VulkanMemoryManager();
    ~VulkanMemoryManager();

    void Init(Ref<VulkanDevice> InDevice);
    void Shutdown();

    Ref<VulkanMemoryAllocation> Alloc(VkDeviceSize AllocationSize, VmaMemoryUsage MemUsage, bool Mappable);
    void Free(Ref<VulkanMemoryAllocation> Allocation);

    uint64 GetTotalMemory(bool bGPUOnly) const;
    void PrintMemInfo() const;

private:
    VmaAllocator GetAllocator()
    {
        return Allocator;
    }

private:
    Ref<VulkanDevice> Device;
    VmaAllocator Allocator;

    VkPhysicalDeviceMemoryProperties MemoryProperties;

    friend VulkanMemoryAllocation;
};

}    // namespace Raphael::RHI

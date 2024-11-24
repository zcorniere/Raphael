#include "VulkanRHI/VulkanMemory.hxx"

#include "Engine/Core/Memory/Memory.hxx"

namespace VulkanRHI
{
VkAllocationCallbacks GAllocationCallbacks;
}

#if VULKAN_CUSTOM_CPU_ALLOCATOR == 1

static FVulkanCPUMemoryManager GVulkanCPUMemMgr;
static std::recursive_mutex GVulkanCPUMemMutex;

FVulkanCPUMemoryManager::FVulkanCPUMemoryManager()
{
    VulkanRHI::GAllocationCallbacks.pUserData = nullptr;
    VulkanRHI::GAllocationCallbacks.pfnAllocation = (PFN_vkAllocationFunction)&FVulkanCPUMemoryManager::Alloc;
    VulkanRHI::GAllocationCallbacks.pfnReallocation = (PFN_vkReallocationFunction)&FVulkanCPUMemoryManager::Realloc;
    VulkanRHI::GAllocationCallbacks.pfnFree = (PFN_vkFreeFunction)&FVulkanCPUMemoryManager::Free;
    VulkanRHI::GAllocationCallbacks.pfnInternalAllocation =
        (PFN_vkInternalAllocationNotification)&FVulkanCPUMemoryManager::InternalAllocationNotification;
    VulkanRHI::GAllocationCallbacks.pfnInternalFree =
        (PFN_vkInternalFreeNotification)&FVulkanCPUMemoryManager::InternalFreeNotification;
}

void* FVulkanCPUMemoryManager::Alloc(void* UserData, size_t Size, size_t Alignment, VkSystemAllocationScope AllocScope)
{
    (void)UserData;
    (void)AllocScope;

    std::scoped_lock Lock(GVulkanCPUMemMutex);
    void* Data = Memory::Malloc(Size, Alignment);
    return Data;
}

void FVulkanCPUMemoryManager::Free(void* UserData, void* Mem)
{
    (void)UserData;
    std::scoped_lock Lock(GVulkanCPUMemMutex);

    Memory::Free(Mem);
}

void* FVulkanCPUMemoryManager::Realloc(void* UserData, void* Original, size_t Size, size_t Alignment,
                                       VkSystemAllocationScope AllocScope)
{
    (void)UserData;
    (void)AllocScope;
    std::scoped_lock Lock(GVulkanCPUMemMutex);

    return Memory::Realloc(Original, Size, Alignment);
}

void FVulkanCPUMemoryManager::InternalAllocationNotification(void* UserData, size_t Size,
                                                             VkInternalAllocationType AllocationType,
                                                             VkSystemAllocationScope AllocScope)
{
    (void)UserData;
    (void)Size;
    (void)AllocationType;
    (void)AllocScope;
}

void FVulkanCPUMemoryManager::InternalFreeNotification(void* UserData, size_t Size,
                                                       VkInternalAllocationType AllocationType,
                                                       VkSystemAllocationScope AllocScope)
{
    (void)UserData;
    (void)Size;
    (void)AllocationType;
    (void)AllocScope;
}

#endif

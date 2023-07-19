#include "VulkanRHI/VulkanMemory.hxx"

#include "Engine/Core/Memory/Memory.hxx"

namespace VulkanRHI
{
VkAllocationCallbacks GAllocationCallbacks;
}

#if VULKAN_CUSTOM_CPU_ALLOCATOR == 1

static VulkanCPUMemoryManager GVulkanCPUMemMgr;
static std::recursive_mutex GVulkanCPUMemMutex;

VulkanCPUMemoryManager::VulkanCPUMemoryManager()
{
    VulkanRHI::GAllocationCallbacks.pUserData = nullptr;
    VulkanRHI::GAllocationCallbacks.pfnAllocation = (PFN_vkAllocationFunction)&VulkanCPUMemoryManager::Alloc;
    VulkanRHI::GAllocationCallbacks.pfnReallocation = (PFN_vkReallocationFunction)&VulkanCPUMemoryManager::Realloc;
    VulkanRHI::GAllocationCallbacks.pfnFree = (PFN_vkFreeFunction)&VulkanCPUMemoryManager::Free;
    VulkanRHI::GAllocationCallbacks.pfnInternalAllocation =
        (PFN_vkInternalAllocationNotification)&VulkanCPUMemoryManager::InternalAllocationNotification;
    VulkanRHI::GAllocationCallbacks.pfnInternalFree =
        (PFN_vkInternalFreeNotification)&VulkanCPUMemoryManager::InternalFreeNotification;
}

void* VulkanCPUMemoryManager::Alloc(void* UserData, size_t Size, size_t Alignment, VkSystemAllocationScope AllocScope)
{
    (void)UserData;
    (void)AllocScope;

    std::scoped_lock Lock(GVulkanCPUMemMutex);
    void* Data = Memory::Malloc(Size, Alignment);
    return Data;
}

void VulkanCPUMemoryManager::Free(void* UserData, void* Mem)
{
    (void)UserData;
    std::scoped_lock Lock(GVulkanCPUMemMutex);

    Memory::Free(Mem);
}

void* VulkanCPUMemoryManager::Realloc(void* UserData, void* Original, size_t Size, size_t Alignment,
                                      VkSystemAllocationScope AllocScope)
{
    (void)UserData;
    (void)AllocScope;

    return Memory::Realloc(Original, Size, Alignment);
}

void VulkanCPUMemoryManager::InternalAllocationNotification(void* UserData, size_t Size,
                                                            VkInternalAllocationType AllocationType,
                                                            VkSystemAllocationScope AllocScope)
{
    (void)UserData;
    (void)Size;
    (void)AllocationType;
    (void)AllocScope;
}

void VulkanCPUMemoryManager::InternalFreeNotification(void* UserData, size_t Size,
                                                      VkInternalAllocationType AllocationType,
                                                      VkSystemAllocationScope AllocScope)
{
    (void)UserData;
    (void)Size;
    (void)AllocationType;
    (void)AllocScope;
}

#endif

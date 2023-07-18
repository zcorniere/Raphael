#include "VulkanRHI/VulkanMemory.hxx"

namespace VulkanRHI
{
VkAllocationCallbacks GAllocationCallbacks;
}

#if VULKAN_CUSTOM_CPU_ALLOCATOR == 1

static VulkanCPUMemoryManager GVulkanCPUMemMgr;

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
    void* Data = std::aligned_alloc(Size, Alignment);
    GVulkanCPUMemMgr.PointerStorageMap[Data] = Size;
    return Data;
}

void VulkanCPUMemoryManager::Free(void* UserData, void* Mem)
{
    (void)UserData;
    GVulkanCPUMemMgr.PointerStorageMap.erase(UserData);
    std::free(Mem);
}

void* VulkanCPUMemoryManager::Realloc(void* UserData, void* Original, size_t Size, size_t Alignment,
                                      VkSystemAllocationScope AllocScope)
{
    (void)UserData;

    void* Data = nullptr;
    if (Size > 0) {
        Alloc(UserData, Size, Alignment, AllocScope);
    }

    if (Original) {
        if (Data) {
            check(GVulkanCPUMemMgr.PointerStorageMap.contains(Original));
            std::memcpy(Data, Original, GVulkanCPUMemMgr.PointerStorageMap.at(Original));
        }
        Free(UserData, Original);
    }
    return Data;
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

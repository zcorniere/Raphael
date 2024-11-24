#pragma once

#include "VulkanRHI/VulkanLoader.hxx"

#if VULKAN_CUSTOM_CPU_ALLOCATOR == 1

class FVulkanCPUMemoryManager
{
public:
    static void* Alloc(void* UserData, size_t Size, size_t Alignment, VkSystemAllocationScope AllocScope);
    static void Free(void* pUserData, void* pMem);
    static void* Realloc(void* pUserData, void* pOriginal, size_t size, size_t alignment,
                         VkSystemAllocationScope allocScope);
    static void InternalAllocationNotification(void* pUserData, size_t size, VkInternalAllocationType allocationType,
                                               VkSystemAllocationScope allocationScope);
    static void InternalFreeNotification(void* pUserData, size_t size, VkInternalAllocationType allocationType,
                                         VkSystemAllocationScope allocationScope);

public:
    VulkanCPUMemoryManager();
};

#endif

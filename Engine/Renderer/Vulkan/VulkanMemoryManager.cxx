#include "Engine/Renderer/Vulkan/VulkanMemoryManager.hxx"

#include "Engine/Misc/Utils.hxx"
#include "Engine/Renderer/Vulkan/VulkanDevice.hxx"
#include "Engine/Renderer/Vulkan/VulkanLoader.hxx"
#include "Engine/Renderer/Vulkan/VulkanUtils.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogVulkanMemoryAllocator, Info);

namespace VulkanRHI
{

void *VulkanMemoryAllocation::Map(VkDeviceSize InSize, VkDeviceSize Offset)
{
    check(bCanBeMapped);
    if (!MappedPointer) {
        check(!MappedPointer);
        checkMsg(InSize + Offset <= Size, "Failed to Map %llu bytes, Offset %llu, AllocSize %llu bytes", InSize, Offset,
                 Size);
        vmaMapMemory(ManagerHandle.GetAllocator(), Allocation, &MappedPointer);
    }
    return MappedPointer;
}
void VulkanMemoryAllocation::Unmap()
{
    check(MappedPointer);
    vmaUnmapMemory(ManagerHandle.GetAllocator(), Allocation);
    MappedPointer = nullptr;
}

void VulkanMemoryAllocation::FlushMappedMemory(VkDeviceSize InOffset, VkDeviceSize InSize)
{
    if (!IsCoherent()) {
        check(IsMapped());
        check(InOffset + InSize <= Size);
        vmaFlushAllocation(ManagerHandle.GetAllocator(), Allocation, InOffset, InSize);
    }
}
void VulkanMemoryAllocation::InvalidateMappedMemory(VkDeviceSize InOffset, VkDeviceSize InSize)
{
    if (!IsCoherent()) {
        check(IsMapped());
        check(InOffset + InSize <= Size);

        vmaInvalidateAllocation(ManagerHandle.GetAllocator(), Allocation, InOffset, InSize);
    }
}

void VulkanMemoryAllocation::BindBuffer(VkBuffer Buffer)
{
    vmaBindBufferMemory(ManagerHandle.GetAllocator(), Allocation, Buffer);
}

void VulkanMemoryAllocation::BindImage(VkImage Image)
{
    vmaBindImageMemory(ManagerHandle.GetAllocator(), Allocation, Image);
}

////////////////////////////////////////////////////////////////////
/// VulkanMemoryManager
////////////////////////////////////////////////////////////////////

VulkanMemoryManager::VulkanMemoryManager(): Allocator(VK_NULL_HANDLE), AllocationCount(0)
{
}

VulkanMemoryManager::~VulkanMemoryManager()
{
}

void VulkanMemoryManager::Init(Ref<VulkanDevice> InDevice)
{
    check(InDevice);
    Device = InDevice;

    Ref<VulkanDynamicRHI> RHI = GetVulkanDynamicRHI();

    VulkanAPI::vkGetPhysicalDeviceMemoryProperties(Device->GetPhysicalHandle(), &MemoryProperties);

    VmaVulkanFunctions Functions;
#define GET_VMA_FUNCTION(Type, Name) \
    Functions.Name = (Type)VulkanAPI::vkGetInstanceProcAddr(RHI->RHIGetVkInstance(), #Name);

    VK_ENTRYPOINTS_VMA(GET_VMA_FUNCTION);
#undef GET_VMA_FUNCTION

    VmaAllocatorCreateInfo CreateInfo{
        .physicalDevice = RHI->RHIGetVkPhysicalDevice(),
        .device = Device->GetInstanceHandle(),
        .pVulkanFunctions = &Functions,
        .instance = RHI->RHIGetVkInstance(),
        .vulkanApiVersion = RHI_VULKAN_VERSION,
    };
    vmaCreateAllocator(&CreateInfo, &Allocator);
    PrintMemInfo();
}

void VulkanMemoryManager::Shutdown()
{
    vmaDestroyAllocator(Allocator);
    Allocator = VK_NULL_HANDLE;
}

Ref<VulkanMemoryAllocation> VulkanMemoryManager::Alloc(VkDeviceSize AllocationSize, VmaMemoryUsage MemUsage,
                                                       bool Mappable)
{
    Ref<VulkanMemoryAllocation> Alloc = Ref<VulkanMemoryAllocation>::Create(*this);
    VmaAllocationCreateFlags flags = 0;
    if (Mappable) flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    VmaAllocationCreateInfo CreateInfo{
        .flags = flags,
        .usage = MemUsage,
    };
    VkMemoryRequirements Requirement{
        .size = AllocationSize,
    };
    vmaAllocateMemory(Allocator, &Requirement, &CreateInfo, &(Alloc->GetHandle()), nullptr);
    AllocationCount += 1;
    return Alloc;
}

void VulkanMemoryManager::Free(Ref<VulkanMemoryAllocation> Allocation)
{
    // Allocator should be removed after this call
    check(Allocation->GetRefCount() == 1);

    vmaFreeMemory(Allocator, Allocation->GetHandle());
    AllocationCount -= 1;
}

uint64 VulkanMemoryManager::GetTotalMemory(bool bGPUOnly) const
{
    uint64 TotalMemory = 0;
    for (uint32 Index = 0; Index < MemoryProperties.memoryHeapCount; ++Index) {
        const bool bIsGPUHeap = ((MemoryProperties.memoryHeaps[Index].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) ==
                                 VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);

        if (bGPUOnly == bIsGPUHeap) { TotalMemory += MemoryProperties.memoryHeaps[Index].size; }
    }
    return TotalMemory;
}

void VulkanMemoryManager::PrintMemInfo() const
{
    LOG(LogVulkanMemoryAllocator, Info, "Max memory allocations {}", Device->GetLimits().maxMemoryAllocationCount);
    LOG(LogVulkanMemoryAllocator, Info, "{} Device Memory Heaps:", MemoryProperties.memoryHeapCount);

    std::vector<VmaBudget> Budgets(MemoryProperties.memoryHeapCount);
    vmaGetHeapBudgets(Allocator, Budgets.data());

    for (unsigned i = 0; i < Budgets.size(); i++) {
        const VmaBudget &b = Budgets.at(i);
        LOG(LogVulkanMemoryAllocator, Info, "{} - VmaBudget.allocationBytes = {}", i,
            Utils::BytesToString(b.statistics.allocationBytes));
        LOG(LogVulkanMemoryAllocator, Info, "{} - VmaBudget.allocationCount = {}", i,
            Utils::BytesToString(b.statistics.allocationCount));
        LOG(LogVulkanMemoryAllocator, Info, "{} - VmaBudget.blockBytes = {}", i,
            Utils::BytesToString(b.statistics.blockBytes));
        LOG(LogVulkanMemoryAllocator, Info, "{} - VmaBudget.blockCount = {}", i,
            Utils::BytesToString(b.statistics.blockCount));
        LOG(LogVulkanMemoryAllocator, Info, "{} - VmaBudget.usage = {}", i, Utils::BytesToString(b.usage));
        LOG(LogVulkanMemoryAllocator, Info, "{} - VmaBudget.budget = {}", i, Utils::BytesToString(b.budget));
    }
    char *JsonString = nullptr;
    vmaBuildStatsString(Allocator, &JsonString, VK_TRUE);
    // write to file
    vmaFreeStatsString(Allocator, JsonString);
}

}    // namespace VulkanRHI

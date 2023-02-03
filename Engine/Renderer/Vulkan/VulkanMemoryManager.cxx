#include "Engine/Renderer/Vulkan/VulkanMemoryManager.hxx"

#include "Engine/Misc/StringUtils.hxx"
#include "Engine/Renderer/Vulkan/VulkanDevice.hxx"
#include "Engine/Renderer/Vulkan/VulkanLoader.hxx"
#include "Engine/Renderer/Vulkan/VulkanUtils.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogVulkanMemoryAllocator, Info);

namespace Raphael::RHI
{

void *VulkanMemoryAllocation::Map(VkDeviceSize InSize, VkDeviceSize Offset)
{
    check(bCanBeMapped);
    if (!MappedPointer) {
        check(!MappedPointer);
        checkMsg(InSize + Offset <= Size, "Failed to Map {} bytes, Offset {}, AllocSize {} bytes", InSize, Offset,
                 Size);
        vmaMapMemory(ManagerHandle->GetAllocator(), Allocation, &MappedPointer);
    }
    return MappedPointer;
}
void VulkanMemoryAllocation::Unmap()
{
    check(MappedPointer);
    vmaUnmapMemory(ManagerHandle->GetAllocator(), Allocation);
    MappedPointer = nullptr;
}

void VulkanMemoryAllocation::FlushMappedMemory(VkDeviceSize InOffset, VkDeviceSize InSize)
{
    if (!IsCoherent()) {
        check(IsMapped());
        check(InOffset + InSize <= Size);
        vmaFlushAllocation(ManagerHandle->GetAllocator(), Allocation, InOffset, InSize);
    }
}
void VulkanMemoryAllocation::InvalidateMappedMemory(VkDeviceSize InOffset, VkDeviceSize InSize)
{
    if (!IsCoherent()) {
        check(IsMapped());
        check(InOffset + InSize <= Size);

        vmaInvalidateAllocation(ManagerHandle->GetAllocator(), Allocation, InOffset, InSize);
    }
}

void VulkanMemoryAllocation::BindBuffer(VkBuffer Buffer)
{
    vmaBindBufferMemory(ManagerHandle->GetAllocator(), Allocation, Buffer);
}

void VulkanMemoryAllocation::BindImage(VkImage Image)
{
    vmaBindImageMemory(ManagerHandle->GetAllocator(), Allocation, Image);
}

////////////////////////////////////////////////////////////////////
/// VulkanMemoryManager
////////////////////////////////////////////////////////////////////

VulkanMemoryManager::VulkanMemoryManager(): Allocator(VK_NULL_HANDLE)
{
}

VulkanMemoryManager::~VulkanMemoryManager()
{
}

void VulkanMemoryManager::Init(Ref<VulkanDevice> InDevice)
{
    check(InDevice);
    Device = InDevice;

    Ref<IVulkanDynamicRHI> RHI = GetIVulkanDynamicRHI();

    VulkanAPI::vkGetPhysicalDeviceMemoryProperties(Device->GetPhysicalHandle(), &MemoryProperties);

    VmaVulkanFunctions Functions;
#define GET_VMA_FUNCTION(Type, Name) \
    Functions.Name = (Type)VulkanAPI::vkGetInstanceProcAddr(RHI->RHIGetVkInstance(), #Name);
    VK_ENTRYPOINTS_VMA(GET_VMA_FUNCTION);

    VmaAllocatorCreateInfo CreateInfo{
        .physicalDevice = RHI->RHIGetVkPhysicalDevice(),
        .device = Device->GetInstanceHandle(),
        .pVulkanFunctions = &Functions,
        .instance = RHI->RHIGetVkInstance(),
        .vulkanApiVersion = VK_API_VERSION_1_2,
    };
#undef GET_VMA_FUNCTION
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
    Ref<VulkanMemoryAllocation> Alloc = Ref<VulkanMemoryAllocation>::Create(this);
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
    return Alloc;
}

void VulkanMemoryManager::Free(Ref<VulkanMemoryAllocation> Allocation)
{
    // Allocator should be removed after this call
    check(Allocation->GetRefCount() == 1);

    vmaFreeMemory(Allocator, Allocation->GetHandle());
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

    for (const VmaBudget &b: Budgets) {
        LOG(LogVulkanMemoryAllocator, Info, "- VmaBudget.allocationBytes = {}",
            Utils::BytesToString(b.statistics.allocationBytes));
        LOG(LogVulkanMemoryAllocator, Info, "- VmaBudget.allocationCount = {}",
            Utils::BytesToString(b.statistics.allocationCount));
        LOG(LogVulkanMemoryAllocator, Info, "- VmaBudget.blockBytes = {}",
            Utils::BytesToString(b.statistics.blockBytes));
        LOG(LogVulkanMemoryAllocator, Info, "- VmaBudget.blockCount = {}",
            Utils::BytesToString(b.statistics.blockCount));
        LOG(LogVulkanMemoryAllocator, Info, "- VmaBudget.usage = {}", Utils::BytesToString(b.usage));
        LOG(LogVulkanMemoryAllocator, Info, "- VmaBudget.budget = {}", Utils::BytesToString(b.budget));
    }
    char *JsonString = nullptr;
    vmaBuildStatsString(Allocator, &JsonString, VK_TRUE);
    // write to file
    vmaFreeStatsString(Allocator, JsonString);
}

}    // namespace Raphael::RHI
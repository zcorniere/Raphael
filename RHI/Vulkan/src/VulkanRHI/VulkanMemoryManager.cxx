#include "VulkanRHI/VulkanMemoryManager.hxx"

#include "Engine/Misc/Utils.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanLoader.hxx"
#include "VulkanRHI/VulkanUtils.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogVulkanMemoryAllocator, Info);

namespace VulkanRHI
{

void *VulkanMemoryAllocation::Map(VkDeviceSize InSize, VkDeviceSize Offset)
{
    check(bCanBeMapped);
    if (!MappedPointer) {
        check(!MappedPointer);
        checkMsg(InSize + Offset <= Size, "Failed to Map {} bytes, Offset {}, AllocSize {} bytes", InSize, Offset,
                 Size);
        VK_CHECK_RESULT(vmaMapMemory(ManagerHandle.GetAllocator(), Allocation, &MappedPointer));
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
        VK_CHECK_RESULT(vmaFlushAllocation(ManagerHandle.GetAllocator(), Allocation, InOffset, InSize));
    }
}
void VulkanMemoryAllocation::InvalidateMappedMemory(VkDeviceSize InOffset, VkDeviceSize InSize)
{
    if (!IsCoherent()) {
        check(IsMapped());
        check(InOffset + InSize <= Size);

        VK_CHECK_RESULT(vmaInvalidateAllocation(ManagerHandle.GetAllocator(), Allocation, InOffset, InSize));
    }
}

void VulkanMemoryAllocation::BindBuffer(VkBuffer Buffer)
{
    VK_CHECK_RESULT(vmaBindBufferMemory(ManagerHandle.GetAllocator(), Allocation, Buffer));
}

void VulkanMemoryAllocation::BindImage(VkImage Image)
{
    VK_CHECK_RESULT(vmaBindImageMemory(ManagerHandle.GetAllocator(), Allocation, Image));
}

////////////////////////////////////////////////////////////////////
/// VulkanMemoryManager
////////////////////////////////////////////////////////////////////

VulkanMemoryManager::VulkanMemoryManager(): Allocator(VK_NULL_HANDLE), AllocationCount(0) {}

VulkanMemoryManager::~VulkanMemoryManager() {}

void VulkanMemoryManager::Init(Ref<VulkanDevice> InDevice)
{
    check(InDevice);
    Device = InDevice;

    Ref<VulkanDynamicRHI> RHI = GetVulkanDynamicRHI();

    VulkanAPI::vkGetPhysicalDeviceMemoryProperties(Device->GetPhysicalHandle(), &MemoryProperties);

    VmaVulkanFunctions Functions;
#define GET_VMA_FUNCTION(Type, Name) \
    Functions.Name = (Type)VulkanAPI::vkGetInstanceProcAddr(RHI->RHIGetVkInstance(), #Name);

#define CHECK_VMA_FUNCTION(Type, Name) \
    if (Functions.Name == nullptr) { LOG(LogVulkanMemoryAllocator, Warning, "Failed to find entry point for " #Name); }

    VK_ENTRYPOINTS_VMA(GET_VMA_FUNCTION);
    VK_ENTRYPOINTS_VMA(CHECK_VMA_FUNCTION);
#undef GET_VMA_FUNCTION
#undef CHECK_VMA_FUNCTION

    VmaAllocatorCreateInfo CreateInfo{
        .physicalDevice = RHI->RHIGetVkPhysicalDevice(),
        .device = Device->GetInstanceHandle(),
        .pVulkanFunctions = &Functions,
        .instance = RHI->RHIGetVkInstance(),
        .vulkanApiVersion = RHI_VULKAN_VERSION,
    };
    VK_CHECK_RESULT(vmaCreateAllocator(&CreateInfo, &Allocator));
    PrintMemInfo();
}

void VulkanMemoryManager::Shutdown()
{

    checkMsg(AllocationCount == 0, "Some memory allocation are still in flight !");
    vmaDestroyAllocator(Allocator);
    Allocator = VK_NULL_HANDLE;
}

Ref<VulkanMemoryAllocation> VulkanMemoryManager::Alloc(const VkMemoryRequirements &MemoryRequirement,
                                                       VmaMemoryUsage MemUsage, bool Mappable)
{
    VmaAllocationCreateInfo CreateInfo = GetCreateInfo(MemUsage, Mappable);
    Ref<VulkanMemoryAllocation> Alloc = Ref<VulkanMemoryAllocation>::Create(*this);
    VK_CHECK_RESULT(vmaAllocateMemory(Allocator, &MemoryRequirement, &CreateInfo, &(Alloc->GetHandle()), nullptr));
    AllocationCount += 1;
    return Alloc;
}

void VulkanMemoryManager::Free(Ref<VulkanMemoryAllocation> &Allocation)
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

VmaAllocationCreateInfo VulkanMemoryManager::GetCreateInfo(VmaMemoryUsage MemUsage, bool Mappable)
{
    VmaAllocationCreateFlags flags = 0;
    if (Mappable) flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    return VmaAllocationCreateInfo{
        .flags = flags,
        .usage = MemUsage,
    };
}

}    // namespace VulkanRHI

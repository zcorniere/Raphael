#include "VulkanRHI/VulkanMemoryManager.hxx"

#include "Engine/Misc/Utils.hxx"
#include "VulkanMemoryManager.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanLoader.hxx"
#include "VulkanRHI/VulkanUtils.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogVulkanMemoryAllocator, Info);

namespace VulkanRHI
{

VulkanMemoryAllocation::VulkanMemoryAllocation(VulkanMemoryManager& InManager)
    : Allocation(VK_NULL_HANDLE),
      AllocationInfo(),
      Size(0),
      MappedPointer(nullptr),
      ManagerHandle(InManager),
      bCanBeMapped(false),
      bIsCoherent(false)
{
}

void VulkanMemoryAllocation::SetName(std::string_view InName)
{
    RObject::SetName(InName);
    if (!InName.empty()) {
        if (Allocation) {
            vmaSetAllocationName(ManagerHandle.GetAllocator(), Allocation, InName.data());
        }
        if (AllocationInfo.deviceMemory) {
            VULKAN_SET_DEBUG_NAME(ManagerHandle.Device, VK_OBJECT_TYPE_DEVICE_MEMORY, AllocationInfo.deviceMemory, "{}", InName);
        }
    }
}

void* VulkanMemoryAllocation::Map(VkDeviceSize InSize, VkDeviceSize Offset)
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

VulkanMemoryManager::VulkanMemoryManager(VulkanDevice* InDevice)
    : Device(InDevice), Allocator(VK_NULL_HANDLE), MemoryProperties(), AllocationCount(0)
{
    check(Device);

    Ref<VulkanDynamicRHI> RHI = GetVulkanDynamicRHI();

    VulkanAPI::vkGetPhysicalDeviceMemoryProperties(Device->GetPhysicalHandle(), &MemoryProperties);

    VmaVulkanFunctions Functions;
    std::memset(&Functions, 0, sizeof(VmaVulkanFunctions));
    Functions.vkGetDeviceProcAddr = VulkanAPI::vkGetDeviceProcAddr;
    Functions.vkGetInstanceProcAddr = VulkanAPI::vkGetInstanceProcAddr;

    VmaAllocatorCreateInfo CreateInfo{
        .physicalDevice = RHI->RHIGetVkPhysicalDevice(),
        .device = Device->GetHandle(),
        .pAllocationCallbacks = VULKAN_CPU_ALLOCATOR,
        .pVulkanFunctions = &Functions,
        .instance = RHI->GetInstance(),
        .vulkanApiVersion = RHI_VULKAN_VERSION,
    };
    VK_CHECK_RESULT(vmaCreateAllocator(&CreateInfo, &Allocator));
    PrintMemInfo();
}

VulkanMemoryManager::~VulkanMemoryManager()
{
    checkMsg(AllocationCount == 0, "Some memory allocation are still in flight !");
    vmaDestroyAllocator(Allocator);
    Allocator = VK_NULL_HANDLE;
}

Ref<VulkanMemoryAllocation> VulkanMemoryManager::Alloc(const VkMemoryRequirements& MemoryRequirement,
                                                       VmaMemoryUsage MemUsage, bool Mappable)
{
    if (MemUsage == VMA_MEMORY_USAGE_GPU_ONLY) {
        checkMsg(!Mappable, "GPU only memory can't be mapped !");
    }
    VmaAllocationCreateInfo CreateInfo = GetCreateInfo(MemUsage, Mappable);
    Ref<VulkanMemoryAllocation> Alloc = Ref<VulkanMemoryAllocation>::Create(*this);
    VK_CHECK_RESULT(
        vmaAllocateMemory(Allocator, &MemoryRequirement, &CreateInfo, &(Alloc->GetHandle()), &Alloc->AllocationInfo));

    AllocationCount += 1;
    return Alloc;
}

void VulkanMemoryManager::Free(Ref<VulkanMemoryAllocation>& Allocation)
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

        if (bGPUOnly == bIsGPUHeap) {
            TotalMemory += MemoryProperties.memoryHeaps[Index].size;
        }
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
        const VmaBudget& b = Budgets.at(i);
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
    char* JsonString = nullptr;
    vmaBuildStatsString(Allocator, &JsonString, VK_TRUE);
    // write to file
    vmaFreeStatsString(Allocator, JsonString);
}

VmaAllocationCreateInfo VulkanMemoryManager::GetCreateInfo(VmaMemoryUsage MemUsage, bool Mappable)
{
    VmaAllocationCreateFlags flags = 0;
    if (Mappable)
        flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    return VmaAllocationCreateInfo{
        .flags = flags,
        .usage = MemUsage,
    };
}

}    // namespace VulkanRHI

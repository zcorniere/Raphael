#include "VulkanRHI/VulkanMemoryManager.hxx"

#include "Engine/Misc/Utils.hxx"
#include "VulkanMemoryManager.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanLoader.hxx"
#include "VulkanRHI/VulkanUtils.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogVulkanMemoryAllocator, Info);

namespace VulkanRHI
{

RVulkanMemoryAllocation::RVulkanMemoryAllocation(FVulkanMemoryManager& InManager)
    : ManagerHandle(InManager)
    , Allocation(VK_NULL_HANDLE)
    , AllocationInfo()
    , Size(0)
    , MappedPointer(nullptr)
    , bCanBeMapped(false)
    , bIsCoherent(false)
{
}

void RVulkanMemoryAllocation::SetName(std::string_view InName)
{
    Super::SetName(InName);
    if (!InName.empty())
    {
        if (Allocation)
        {
            vmaSetAllocationName(ManagerHandle.Allocator, Allocation, InName.data());
        }
        if (AllocationInfo.deviceMemory)
        {
            VULKAN_SET_DEBUG_NAME(ManagerHandle.Device, VK_OBJECT_TYPE_DEVICE_MEMORY, AllocationInfo.deviceMemory,
                                  "{:s}", InName);
        }
    }
}

void* RVulkanMemoryAllocation::Map(VkDeviceSize InSize, VkDeviceSize Offset)
{
    check(bCanBeMapped);
    if (AllocationInfo.pMappedData)
    {
        return AllocationInfo.pMappedData;
    }
    if (!MappedPointer)
    {
        check(!MappedPointer);
        checkMsg(InSize + Offset <= Size, "Can't to Map {} bytes, Offset {}, AllocSize {} bytes", InSize, Offset, Size);
        VK_CHECK_RESULT(vmaMapMemory(ManagerHandle.Allocator, Allocation, &MappedPointer));
    }
    return MappedPointer;
}

void RVulkanMemoryAllocation::Unmap()
{
    if (MappedPointer)
    {
        vmaUnmapMemory(ManagerHandle.Allocator, Allocation);
        MappedPointer = nullptr;
    }
}

void RVulkanMemoryAllocation::FlushMappedMemory(VkDeviceSize InOffset, VkDeviceSize InSize)
{
    if (!IsCoherent())
    {
        check(IsMapped());
        check(InOffset + InSize <= Size);
        VK_CHECK_RESULT(vmaFlushAllocation(ManagerHandle.Allocator, Allocation, InOffset, InSize));
    }
}
void RVulkanMemoryAllocation::InvalidateMappedMemory(VkDeviceSize InOffset, VkDeviceSize InSize)
{
    if (!IsCoherent())
    {
        check(IsMapped());
        check(InOffset + InSize <= Size);

        VK_CHECK_RESULT(vmaInvalidateAllocation(ManagerHandle.Allocator, Allocation, InOffset, InSize));
    }
}

void RVulkanMemoryAllocation::BindBuffer(VkBuffer Buffer)
{
    VK_CHECK_RESULT(vmaBindBufferMemory(ManagerHandle.Allocator, Allocation, Buffer));
}

void RVulkanMemoryAllocation::BindImage(VkImage Image)
{
    VK_CHECK_RESULT(vmaBindImageMemory(ManagerHandle.Allocator, Allocation, Image));
}

////////////////////////////////////////////////////////////////////
/// VulkanMemoryManager
////////////////////////////////////////////////////////////////////

FVulkanMemoryManager::FVulkanMemoryManager(FVulkanDevice* InDevice)
    : IDeviceChild(InDevice)
    , Allocator(VK_NULL_HANDLE)
    , MemoryProperties()
    , AllocationCount(0)
{
    check(Device);

    FVulkanDynamicRHI* RHI = GetVulkanDynamicRHI();

    VulkanAPI::vkGetPhysicalDeviceMemoryProperties(Device->GetPhysicalHandle(), &MemoryProperties);

    VmaVulkanFunctions Functions;
    std::memset(&Functions, 0, sizeof(VmaVulkanFunctions));
    Functions.vkGetDeviceProcAddr = VulkanAPI::vkGetDeviceProcAddr;
    Functions.vkGetInstanceProcAddr = VulkanAPI::vkGetInstanceProcAddr;
    Functions.vkAllocateMemory = VulkanAPI::vkAllocateMemory;
    Functions.vkBindBufferMemory = VulkanAPI::vkBindBufferMemory;
    Functions.vkBindImageMemory = VulkanAPI::vkBindImageMemory;
    Functions.vkCreateBuffer = VulkanAPI::vkCreateBuffer;
    Functions.vkCreateImage = VulkanAPI::vkCreateImage;
    Functions.vkDestroyBuffer = VulkanAPI::vkDestroyBuffer;
    Functions.vkDestroyImage = VulkanAPI::vkDestroyImage;
    Functions.vkFlushMappedMemoryRanges = VulkanAPI::vkFlushMappedMemoryRanges;
    Functions.vkFreeMemory = VulkanAPI::vkFreeMemory;
    Functions.vkGetBufferMemoryRequirements = VulkanAPI::vkGetBufferMemoryRequirements;
    Functions.vkGetImageMemoryRequirements = VulkanAPI::vkGetImageMemoryRequirements;
    Functions.vkGetPhysicalDeviceMemoryProperties = VulkanAPI::vkGetPhysicalDeviceMemoryProperties;
    Functions.vkGetPhysicalDeviceProperties = VulkanAPI::vkGetPhysicalDeviceProperties;
    Functions.vkMapMemory = VulkanAPI::vkMapMemory;
    Functions.vkUnmapMemory = VulkanAPI::vkUnmapMemory;

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

FVulkanMemoryManager::~FVulkanMemoryManager()
{
    const unsigned AllocationCountLocal = AllocationCount.load();
    if (AllocationCountLocal > 0)
    {
        LOG(LogVulkanMemoryAllocator, Error, "Some memory allocation ({}) are still in flight !", AllocationCountLocal);

#ifndef NDEBUG
        std::unique_lock Lock(MemoryAllocationArrayMutex);

        for (WeakRef<RVulkanMemoryAllocation>& Alloc: MemoryAllocationArray)
        {
            if (Alloc.IsValid())
            {
                LOG(LogVulkanMemoryAllocator, Error, "Allocation: \"{:s}\" ({})", Alloc->GetName(),
                    Alloc->GetRefCount());
            }
            else
            {
                LOG(LogVulkanMemoryAllocator, Error, "Allocation: nullptr");
            }
        }
#endif    // !NDEBUG
    }
    checkMsg(AllocationCount == 0, "Some memory allocation ({}) are still in flight !", AllocationCount.load());
    vmaDestroyAllocator(Allocator);
    Allocator = VK_NULL_HANDLE;
}

Ref<RVulkanMemoryAllocation> FVulkanMemoryManager::Alloc(const VkMemoryRequirements& MemoryRequirement,
                                                         VmaMemoryUsage MemUsage, bool Mappable)
{
    if (MemUsage == VMA_MEMORY_USAGE_GPU_ONLY)
    {
        checkMsg(!Mappable, "GPU only memory can't be mapped !");
    }
    VmaAllocationCreateInfo CreateInfo = GetCreateInfo(MemUsage, Mappable);
    Ref<RVulkanMemoryAllocation> Alloc = Ref<RVulkanMemoryAllocation>::Create(*this);

#ifndef NDEBUG
    {
        std::unique_lock Lock(MemoryAllocationArrayMutex);
        MemoryAllocationArray.Add(Alloc);
    }
#endif    // !NDEBUG

    VK_CHECK_RESULT(
        vmaAllocateMemory(Allocator, &MemoryRequirement, &CreateInfo, &(Alloc->GetHandle()), &Alloc->AllocationInfo));
    Alloc->Size = MemoryRequirement.size;
    Alloc->bCanBeMapped = Mappable;

    VkMemoryPropertyFlags memPropFlags;
    vmaGetAllocationMemoryProperties(Allocator, Alloc->GetHandle(), &memPropFlags);
    Alloc->bIsCoherent = memPropFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    Alloc->bCanBeMapped = memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

    AllocationCount += 1;
    return Alloc;
}

std::pair<VkBuffer, Ref<RVulkanMemoryAllocation>>
FVulkanMemoryManager::Alloc(const VkBufferCreateInfo& BufferCreateInfo, const VmaAllocationCreateInfo& AllocCreateInfo)
{
    Ref<RVulkanMemoryAllocation> Alloc = Ref<RVulkanMemoryAllocation>::Create(*this);

#ifndef NDEBUG
    {
        std::unique_lock Lock(MemoryAllocationArrayMutex);
        MemoryAllocationArray.Add(Alloc);
    }
#endif    // NDEBUG

    VkBuffer Buffer = VK_NULL_HANDLE;
    VK_CHECK_RESULT(vmaCreateBuffer(Allocator, &BufferCreateInfo, &AllocCreateInfo, &Buffer, &(Alloc->GetHandle()),
                                    &Alloc->AllocationInfo));

    VkMemoryPropertyFlags memPropFlags;
    vmaGetAllocationMemoryProperties(Allocator, Alloc->GetHandle(), &memPropFlags);

    Alloc->Size = BufferCreateInfo.size;
    Alloc->bCanBeMapped = memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    Alloc->bIsCoherent = memPropFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    AllocationCount += 1;
    return {Buffer, Alloc};
}

void FVulkanMemoryManager::Free(Ref<RVulkanMemoryAllocation>& Allocation)
{
    // Allocator should be removed after this call
    check(Allocation->GetRefCount() == 1);

#ifndef NDEBUG
    {
        std::unique_lock Lock(MemoryAllocationArrayMutex);
        MemoryAllocationArray.Remove(Allocation);
    }
#endif    // NDEBUG

    vmaFreeMemory(Allocator, Allocation->GetHandle());
    AllocationCount -= 1;
}

uint64 FVulkanMemoryManager::GetTotalMemory(bool bGPUOnly) const
{
    uint64 TotalMemory = 0;
    for (uint32 Index = 0; Index < MemoryProperties.memoryHeapCount; ++Index)
    {
        const bool bIsGPUHeap = ((MemoryProperties.memoryHeaps[Index].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) ==
                                 VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);

        if (bGPUOnly == bIsGPUHeap)
        {
            TotalMemory += MemoryProperties.memoryHeaps[Index].size;
        }
    }
    return TotalMemory;
}

void FVulkanMemoryManager::PrintMemInfo() const
{
    LOG(LogVulkanMemoryAllocator, Info, "Max memory allocations {}", Device->GetLimits().maxMemoryAllocationCount);
    LOG(LogVulkanMemoryAllocator, Info, "{} Device Memory Heaps:", MemoryProperties.memoryHeapCount);

    TArray<VmaBudget> Budgets(MemoryProperties.memoryHeapCount);
    vmaGetHeapBudgets(Allocator, Budgets.Raw());

    for (unsigned i = 0; i < Budgets.Size(); i++)
    {
        const VmaBudget& b = Budgets[i];
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

VmaAllocationCreateInfo FVulkanMemoryManager::GetCreateInfo(VmaMemoryUsage MemUsage, bool Mappable)
{
    VmaAllocationCreateFlags flags = 0;
    if (Mappable)
        flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    return VmaAllocationCreateInfo{
        .flags = flags,
        .usage = MemUsage,
    };
}

std::string FVulkanMemoryManager::GetVMADumpString() const
{
    char* String = nullptr;
    vmaBuildStatsString(Allocator, &String, VK_TRUE);
    std::string Result(String);
    vmaFreeStatsString(Allocator, String);
    return Result;
}

}    // namespace VulkanRHI

#include "Engine/Renderer/Vulkan/VulkanAllocator.hxx"

#include "Engine/Misc/StringUtils.hxx"
#include "Engine/Renderer/Vulkan/VulkanContext.hxx"

#if defined(RPH_LOG_ALLOCATION)
    #define LOG_ALLOCATION(...) LOG(LogVulkanAllocator, Trace, __VA_ARGS__)
#else
    #define LOG_ALLOCATION(...)
#endif

namespace Raphael
{

DECLARE_LOGGER_CATEGORY(Core, LogVulkanAllocator, Trace);

struct VulkanAllocatorData {
    VmaAllocator Allocator;
    uint64_t TotalAllocatedBytes = 0;
};

static VulkanAllocatorData *s_Data = nullptr;

VulkanAllocator::VulkanAllocator(const std::string &tag): m_Tag(tag)
{
}

VulkanAllocator::~VulkanAllocator()
{
}

VmaAllocation VulkanAllocator::AllocateBuffer(VkBufferCreateInfo bufferCreateInfo, VmaMemoryUsage usage,
                                              VkBuffer &outBuffer)
{
    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = usage;

    VmaAllocation allocation;
    vmaCreateBuffer(s_Data->Allocator, &bufferCreateInfo, &allocCreateInfo, &outBuffer, &allocation, nullptr);

    // TODO: Tracking
    VmaAllocationInfo allocInfo{};
    vmaGetAllocationInfo(s_Data->Allocator, allocation, &allocInfo);
    LOG_ALLOCATION("VulkanAllocator ({0}): allocating buffer; size = {1}", m_Tag, Utils::BytesToString(allocInfo.size));

    {
        s_Data->TotalAllocatedBytes += allocInfo.size;
        LOG_ALLOCATION("VulkanAllocator ({0}): total allocated since start is {1}", m_Tag,
                       Utils::BytesToString(s_Data->TotalAllocatedBytes));
    }

    return allocation;
}

VmaAllocation VulkanAllocator::AllocateImage(VkImageCreateInfo imageCreateInfo, VmaMemoryUsage usage, VkImage &outImage)
{
    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = usage;

    VmaAllocation allocation;
    vmaCreateImage(s_Data->Allocator, &imageCreateInfo, &allocCreateInfo, &outImage, &allocation, nullptr);

    // TODO: Tracking
    VmaAllocationInfo allocInfo;
    vmaGetAllocationInfo(s_Data->Allocator, allocation, &allocInfo);
    LOG_ALLOCATION("VulkanAllocator ({0}): allocating image; size = {1}", m_Tag, Utils::BytesToString(allocInfo.size));

    {
        s_Data->TotalAllocatedBytes += allocInfo.size;
        LOG_ALLOCATION("VulkanAllocator ({0}): total allocated since start is {1}", m_Tag,
                       Utils::BytesToString(s_Data->TotalAllocatedBytes));
    }
    return allocation;
}

void VulkanAllocator::Free(VmaAllocation allocation)
{
    vmaFreeMemory(s_Data->Allocator, allocation);
}

void VulkanAllocator::DestroyImage(VkImage image, VmaAllocation allocation)
{
    check(image);
    check(allocation);
    vmaDestroyImage(s_Data->Allocator, image, allocation);
}

void VulkanAllocator::DestroyBuffer(VkBuffer buffer, VmaAllocation allocation)
{
    check(buffer);
    check(allocation);
    vmaDestroyBuffer(s_Data->Allocator, buffer, allocation);
}

void VulkanAllocator::UnmapMemory(VmaAllocation allocation)
{
    vmaUnmapMemory(s_Data->Allocator, allocation);
}

void VulkanAllocator::DumpStats()
{
    const auto &memoryProps = VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->GetMemoryProperties();
    std::vector<VmaBudget> budgets(memoryProps.memoryHeapCount);
    vmaGetHeapBudgets(s_Data->Allocator, budgets.data());

    LOG(LogVulkanAllocator, Warn, "-----------------------------------");
    for (VmaBudget &b: budgets) {
        LOG(LogVulkanAllocator, Warn, "VmaBudget.allocationBytes = {0}",
            Utils::BytesToString(b.statistics.allocationBytes));
        LOG(LogVulkanAllocator, Warn, "VmaBudget.blockBytes = {0}", Utils::BytesToString(b.statistics.blockBytes));
        LOG(LogVulkanAllocator, Warn, "VmaBudget.usage = {0}", Utils::BytesToString(b.usage));
        LOG(LogVulkanAllocator, Warn, "VmaBudget.budget = {0}", Utils::BytesToString(b.budget));
    }
    LOG(LogVulkanAllocator, Warn, "-----------------------------------");
}

GPUMemoryStats VulkanAllocator::GetStats()
{
    const auto &memoryProps = VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->GetMemoryProperties();
    std::vector<VmaBudget> budgets(memoryProps.memoryHeapCount);
    vmaGetHeapBudgets(s_Data->Allocator, budgets.data());

    uint64_t usage = 0;
    uint64_t budget = 0;

    for (VmaBudget &b: budgets) {
        usage += b.usage;
        budget += b.budget;
    }

    // Ternary because budget can somehow be smaller than usage.
    return {usage, budget > usage ? budget - usage : 0ull};
}

void VulkanAllocator::Init(Ref<VulkanDevice> device)
{
    s_Data = new VulkanAllocatorData();

    // Initialize VulkanMemoryAllocator
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
    allocatorInfo.physicalDevice = device->GetPhysicalDevice()->GetVulkanPhysicalDevice();
    allocatorInfo.device = device->GetVulkanDevice();
    allocatorInfo.instance = VulkanContext::GetInstance();

    vmaCreateAllocator(&allocatorInfo, &s_Data->Allocator);
}

void VulkanAllocator::Shutdown()
{
    vmaDestroyAllocator(s_Data->Allocator);

    delete s_Data;
    s_Data = nullptr;
}

VmaAllocator &VulkanAllocator::GetVMAAllocator()
{
    return s_Data->Allocator;
}

}    // namespace Raphael

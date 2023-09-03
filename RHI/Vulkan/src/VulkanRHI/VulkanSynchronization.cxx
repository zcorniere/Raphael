#include "VulkanRHI/VulkanSynchronization.hxx"
#include "VulkanRHI/VulkanDevice.hxx"

#include "VulkanRHI/VulkanUtils.hxx"

namespace
{
static VkAccessFlags GetVkAccessMaskForLayout(const VkImageLayout Layout)
{
    VkAccessFlags Flags = 0;

    switch (Layout) {
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            Flags = VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            Flags = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            Flags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
            Flags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            Flags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            Flags = VK_ACCESS_SHADER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
            Flags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            Flags = 0;
            break;

        case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
            Flags = VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;
            break;

        case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
            Flags = VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
            break;

        case VK_IMAGE_LAYOUT_GENERAL:
            // todo-jn: could be used for R64 in read layout
        case VK_IMAGE_LAYOUT_UNDEFINED:
            Flags = 0;
            break;

        case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
            // todo-jn: sync2 currently only used by depth/stencil targets
            Flags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
            // todo-jn: sync2 currently only used by depth/stencil targets
            Flags = VK_ACCESS_SHADER_READ_BIT;
            break;

        default:
            checkNoEntry();
            break;
    }

    return Flags;
}

static VkPipelineStageFlags GetVkStageFlagsForLayout(VkImageLayout Layout)
{
    VkPipelineStageFlags Flags = 0;

    switch (Layout) {
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            Flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            Flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            Flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
            Flags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            Flags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
            Flags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;

        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            Flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            break;

        case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
            Flags = VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT;
            break;

        case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
            Flags = VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
            break;

        case VK_IMAGE_LAYOUT_GENERAL:
        case VK_IMAGE_LAYOUT_UNDEFINED:
            Flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            break;

        case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
            Flags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;

        case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
            Flags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;

        default:
            break;
    }

    return Flags;
}
}    // namespace

void VulkanRHI::VulkanSetImageLayout(VkCommandBuffer CmdBuffer, VkImage Image, VkImageLayout OldLayout,
                                     VkImageLayout NewLayout, const VkImageSubresourceRange& SubresourceRange)
{
    VulkanRHI::Barrier Barrier;
    Barrier.TransitionLayout(Image, OldLayout, NewLayout, SubresourceRange);
    Barrier.Execute(CmdBuffer);
}

namespace VulkanRHI
{

VkImageSubresourceRange Barrier::MakeSubresourceRange(VkImageAspectFlags AspectMask, uint32 FirstMip, uint32 NumMips,
                                                      uint32 FirstLayer, uint32 NumLayers)
{
    VkImageSubresourceRange Range;
    Range.aspectMask = AspectMask;
    Range.baseMipLevel = FirstMip;
    Range.levelCount = NumMips;
    Range.baseArrayLayer = FirstLayer;
    Range.layerCount = NumLayers;
    return Range;
}

Barrier::Barrier()
{
}

void Barrier::TransitionLayout(VkImage Image, VkImageLayout OldLayout, VkImageLayout NewLayout,
                               const VkImageSubresourceRange& SubresourceRange)
{
    VkImageMemoryBarrier& Barrier = ImageBarrier.emplace_back();

    std::memset(&Barrier, 0, sizeof(Barrier));
    Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    Barrier.srcAccessMask = GetVkAccessMaskForLayout(OldLayout);
    Barrier.dstAccessMask = GetVkAccessMaskForLayout(NewLayout);
    Barrier.oldLayout = OldLayout;
    Barrier.newLayout = NewLayout;
    Barrier.image = Image;
    Barrier.subresourceRange = SubresourceRange;
}

void Barrier::Execute(VkCommandBuffer CmdBuffer)
{
    VkPipelineStageFlags SrcStageMask = 0;
    VkPipelineStageFlags DstStageMask = 0;

    for (const auto& Barrier: ImageBarrier) {
        SrcStageMask |= GetVkStageFlagsForLayout(Barrier.oldLayout);
        DstStageMask |= GetVkStageFlagsForLayout(Barrier.newLayout);
    }
    if (!ImageBarrier.empty()) {
        VulkanAPI::vkCmdPipelineBarrier(CmdBuffer, SrcStageMask, DstStageMask, 0, 0, nullptr, 0, nullptr,
                                        ImageBarrier.size(), ImageBarrier.data());
    }
}

Semaphore::Semaphore(VulkanDevice* InDevice): Device(InDevice), SemaphoreHandle(VK_NULL_HANDLE)
{
    VkSemaphoreCreateInfo CreateInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    VK_CHECK_RESULT(
        VulkanAPI::vkCreateSemaphore(Device->GetHandle(), &CreateInfo, VULKAN_CPU_ALLOCATOR, &SemaphoreHandle));
}

Semaphore::~Semaphore()
{
    if (SemaphoreHandle) {
        VulkanAPI::vkDestroySemaphore(Device->GetHandle(), SemaphoreHandle, VULKAN_CPU_ALLOCATOR);
    }
    SemaphoreHandle = VK_NULL_HANDLE;
}

void Semaphore::SetName(std::string_view InName)
{
    RObject::SetName(InName);
    VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_SEMAPHORE, SemaphoreHandle, "{:s}.Semaphore", InName);
}

Fence::Fence(VulkanDevice* InDevice, bool bCreateSignaled)
    : Device(InDevice), State(bCreateSignaled ? Fence::State::Signaled : Fence::State::NotReady)
{
    VkFenceCreateInfo Info{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = 0,
    };
    if (bCreateSignaled) {
        Info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }
    VK_CHECK_RESULT(VulkanAPI::vkCreateFence(Device->GetHandle(), &Info, VULKAN_CPU_ALLOCATOR, &Handle));
}

Fence::~Fence()
{
    VulkanAPI::vkDestroyFence(Device->GetHandle(), Handle, VULKAN_CPU_ALLOCATOR);
}

void Fence::SetName(std::string_view InName)
{
    RObject::SetName(InName);
    VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_FENCE, Handle, "{:s}.Fence", InName);
}

void Fence::Reset()
{
    if (State != State::NotReady) {
        VK_CHECK_RESULT(VulkanAPI::vkResetFences(Device->GetHandle(), 1, &Handle));
        State = State::NotReady;
    }
}

bool Fence::Wait(uint64 TimeInNanoseconds)
{
    check(State == State::NotReady);

    VkResult Result = VulkanAPI::vkWaitForFences(Device->GetHandle(), 1, &Handle, true, TimeInNanoseconds);
    switch (Result) {
        case VK_SUCCESS:
            State = State::Signaled;
            return true;
        case VK_TIMEOUT:
            break;
        default:
            VK_CHECK_RESULT_EXPANDED(Result);
            break;
    }
    return false;
}

bool Fence::CheckFenceStatus()
{
    check(State == State::NotReady);
    VkResult Result = VulkanAPI::vkGetFenceStatus(Device->GetHandle(), Handle);
    switch (Result) {
        case VK_SUCCESS:
            State = State::Signaled;
            return true;

        case VK_NOT_READY:
            break;

        default:
            VK_CHECK_RESULT(Result);
            break;
    }

    return false;
}

}    // namespace VulkanRHI

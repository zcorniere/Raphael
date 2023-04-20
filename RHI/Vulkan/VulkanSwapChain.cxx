#include "RHI/Vulkan/VulkanSwapChain.hxx"

#include "RHI/Vulkan/VulkanDevice.hxx"
#include "RHI/Vulkan/VulkanLoader.hxx"
#include "RHI/Vulkan/VulkanPlatform.hxx"
#include "RHI/Vulkan/VulkanQueue.hxx"
#include "RHI/Vulkan/VulkanSynchronization.hxx"
#include "RHI/Vulkan/VulkanUtils.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogVulkanSwapchain, Info);

namespace VulkanRHI
{

VulkanSwapChain::SupportDetails VulkanSwapChain::SupportDetails::QuerySwapChainSupport(const Ref<VulkanDevice> &Device,
                                                                                       const VkSurfaceKHR &Surface)
{
    VulkanSwapChain::SupportDetails Details;

    {
        VK_CHECK_RESULT(VulkanAPI::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device->GetPhysicalHandle(), Surface,
                                                                             &Details.Capabilities));
    }

    {
        uint32 NumFormats;
        VK_CHECK_RESULT(VulkanAPI::vkGetPhysicalDeviceSurfaceFormatsKHR(Device->GetPhysicalHandle(), Surface,
                                                                        &NumFormats, nullptr));
        check(NumFormats > 0);

        Details.Formats.resize(NumFormats);
        VK_CHECK_RESULT(VulkanAPI::vkGetPhysicalDeviceSurfaceFormatsKHR(Device->GetPhysicalHandle(), Surface,
                                                                        &NumFormats, Details.Formats.data()));
    }

    {
        uint32 NumFormats;
        VK_CHECK_RESULT(VulkanAPI::vkGetPhysicalDeviceSurfacePresentModesKHR(Device->GetPhysicalHandle(), Surface,
                                                                             &NumFormats, nullptr));
        check(NumFormats > 0);

        Details.PresentModes.resize(NumFormats);
        VK_CHECK_RESULT(VulkanAPI::vkGetPhysicalDeviceSurfacePresentModesKHR(Device->GetPhysicalHandle(), Surface,
                                                                             &NumFormats, Details.PresentModes.data()));
    }
    return Details;
}

VkSurfaceFormatKHR VulkanSwapChain::SupportDetails::ChooseSwapSurfaceFormat() const noexcept
{
    for (const auto &availableFormat: Formats)
        if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;
    return Formats.at(0);
}

VkPresentModeKHR VulkanSwapChain::SupportDetails::ChooseSwapPresentMode(bool LockToVSync) const noexcept
{
    bool bFoundPresentModeMailbox = false;
    bool bFoundPresentModeImmediate = false;
    bool bFoundPresentModeFIFO = false;

    LOG(LogVulkanSwapchain, Info, "Found {} Surface present modes:", PresentModes.size());

    for (const auto &availablePresentMode: PresentModes) {
        switch (availablePresentMode) {
            case VK_PRESENT_MODE_MAILBOX_KHR:
                bFoundPresentModeMailbox = true;
                LOG(LogVulkanSwapchain, Info, "- VK_PRESENT_MODE_MAILBOX_KHR ({})", static_cast<std::underlying_type_t<VkPresentModeKHR>>(VK_PRESENT_MODE_MAILBOX_KHR));
                break;
            case VK_PRESENT_MODE_IMMEDIATE_KHR:
                bFoundPresentModeImmediate = true;
                LOG(LogVulkanSwapchain, Info, "- VK_PRESENT_MODE_IMMEDIATE_KHR ({})", static_cast<std::underlying_type_t<VkPresentModeKHR>>(VK_PRESENT_MODE_IMMEDIATE_KHR));
                break;
            case VK_PRESENT_MODE_FIFO_KHR:
                bFoundPresentModeFIFO = true;
                LOG(LogVulkanSwapchain, Info, "- VK_PRESENT_MODE_FIFO_KHR ({})", static_cast<std::underlying_type_t<VkPresentModeKHR>>(VK_PRESENT_MODE_FIFO_KHR));
                break;
            case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
                LOG(LogVulkanSwapchain, Info, "- VK_PRESENT_MODE_FIFO_RELAXED_KHR ({})",
                    static_cast<std::underlying_type_t<VkPresentModeKHR>>(VK_PRESENT_MODE_FIFO_RELAXED_KHR));
                break;
            default: LOG(LogVulkanSwapchain, Info, "- VkPresentModeKHR {}", static_cast<std::underlying_type_t<VkPresentModeKHR>>(VK_PRESENT_MODE_FIFO_RELAXED_KHR)); break;
        }
    }

    if (bFoundPresentModeImmediate && !LockToVSync) {
        return VK_PRESENT_MODE_IMMEDIATE_KHR;
    } else if (bFoundPresentModeMailbox) {
        return VK_PRESENT_MODE_MAILBOX_KHR;
    } else if (bFoundPresentModeFIFO) {
        return VK_PRESENT_MODE_FIFO_KHR;
    } else {
        LOG(LogVulkanSwapchain, Warn, "Couldn't find desired PresentMode! Using {}",
            VK_TYPE_TO_STRING(VkPresentModeKHR, PresentModes[0]));
        return PresentModes[0];
    }
}

VkExtent2D VulkanSwapChain::SupportDetails::ChooseSwapExtent(const glm::uvec2 &s) const noexcept
{
    if (Capabilities.currentExtent.width != UINT32_MAX) {
        return Capabilities.currentExtent;
    } else {
        VkExtent2D size{
            .width = s.x,
            .height = s.y,
        };
        size.width = std::clamp(size.width, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width);
        size.height = std::clamp(size.height, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height);
        return size;
    }
}

VulkanSwapChain::VulkanSwapChain(VkInstance InInstance, Ref<VulkanDevice> &InDevice, void *WindowHandle,
                                 glm::uvec2 Size, uint32 InDesiredNumBackBuffers, std::vector<VkImage> &OutImages,
                                 bool LockToVSync, VulkanSwapChainRecreateInfo *RecreateInfo)
    : Device(InDevice), CurrentImageIndex(0), SwapChain(VK_NULL_HANDLE), Surface(VK_NULL_HANDLE), Instance(InInstance)
{
    if (RecreateInfo && RecreateInfo->SwapChain != VK_NULL_HANDLE) {
        check(RecreateInfo->Surface != VK_NULL_HANDLE);
        Surface = RecreateInfo->Surface;
        RecreateInfo->Surface = VK_NULL_HANDLE;
    } else {
        VulkanPlatform::CreateSurface(WindowHandle, Instance, &Surface);
    }

    const SupportDetails SwapChainSupport = SupportDetails::QuerySwapChainSupport(Device, Surface);
    VkSurfaceFormatKHR SurfaceFormat = SwapChainSupport.ChooseSwapSurfaceFormat();
    VkPresentModeKHR PresentMode = SwapChainSupport.ChooseSwapPresentMode(LockToVSync);
    VkExtent2D Extent = SwapChainSupport.ChooseSwapExtent(Size);

    uint32 ImageCount = std::max(SwapChainSupport.Capabilities.minImageCount + 1, InDesiredNumBackBuffers);
    if (SwapChainSupport.Capabilities.maxImageCount > 0 && ImageCount > SwapChainSupport.Capabilities.maxImageCount) {
        ImageCount = SwapChainSupport.Capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR CreateInfo{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .surface = Surface,
        .minImageCount = ImageCount,
        .imageFormat = SurfaceFormat.format,
        .imageColorSpace = SurfaceFormat.colorSpace,
        .imageExtent = Extent,
        .imageArrayLayers = 1,
        .imageUsage =
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = SwapChainSupport.Capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = PresentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = (RecreateInfo) ? (RecreateInfo->SwapChain) : (VK_NULL_HANDLE),
    };

    LOG(LogVulkanSwapchain, Info, "Creating new VK swapchain with {}, {}, {}, num images {}",
        VK_TYPE_TO_STRING(VkPresentModeKHR, CreateInfo.presentMode),
        VK_TYPE_TO_STRING(VkFormat, CreateInfo.imageFormat),
        VK_TYPE_TO_STRING(VkColorSpaceKHR, CreateInfo.imageColorSpace), static_cast<uint32>(CreateInfo.minImageCount));

    VK_CHECK_RESULT_EXPANDED(
        VulkanPlatform::CreateSwapchainKHR(Device->GetInstanceHandle(), &CreateInfo, nullptr, &SwapChain));

    ImageFormat = SurfaceFormat.format;
    if (RecreateInfo) {
        if (RecreateInfo->SwapChain != VK_NULL_HANDLE) {
            VulkanPlatform::DestroySwapchainKHR(Device->GetInstanceHandle(), RecreateInfo->SwapChain, nullptr);
            RecreateInfo->SwapChain = VK_NULL_HANDLE;
        }
        if (RecreateInfo->Surface != VK_NULL_HANDLE) {
            VulkanAPI::vkDestroySurfaceKHR(Instance, RecreateInfo->Surface, nullptr);
            RecreateInfo->Surface = VK_NULL_HANDLE;
        }
    }

    InternalSize.x = std::min(Size.x, CreateInfo.imageExtent.width);
    InternalSize.y = std::min(Size.y, CreateInfo.imageExtent.height);

    uint32 NumSwapchainImages;
    VK_CHECK_RESULT_EXPANDED(
        VulkanAPI::vkGetSwapchainImagesKHR(Device->GetInstanceHandle(), SwapChain, &NumSwapchainImages, nullptr));
    OutImages.resize(NumSwapchainImages);
    VK_CHECK_RESULT_EXPANDED(VulkanAPI::vkGetSwapchainImagesKHR(Device->GetInstanceHandle(), SwapChain,
                                                                &NumSwapchainImages, OutImages.data()));

    ImageAcquiredSemaphore.resize(NumSwapchainImages);
    for (uint32 BufferIndex = 0; BufferIndex < NumSwapchainImages; BufferIndex++) {
        ImageAcquiredSemaphore[BufferIndex] = Ref<Semaphore>::Create(Device);
    }

    ImageInUseFence.resize(NumSwapchainImages);
    for (uint32 BufferIndex = 0; BufferIndex < NumSwapchainImages; BufferIndex++) {
        ImageInUseFence[BufferIndex] = Ref<Fence>::Create(Device, false);
    }
}

void VulkanSwapChain::Destroy(VulkanSwapChainRecreateInfo *RecreateInfo)
{
    if (RecreateInfo) {
        RecreateInfo->SwapChain = SwapChain;
        RecreateInfo->Surface = Surface;
    } else {
        VulkanAPI::vkDestroySwapchainKHR(Device->GetInstanceHandle(), SwapChain, nullptr);
        VulkanAPI::vkDestroySurfaceKHR(Instance, Surface, nullptr);
    }
    Surface = VK_NULL_HANDLE;
    SwapChain = VK_NULL_HANDLE;
    ImageAcquiredSemaphore.clear();
    ImageInUseFence.clear();
}

VulkanSwapChain::Status VulkanSwapChain::Present(Ref<VulkanQueue> &PresentQueue, Ref<Semaphore> &RenderingComplete)
{
    check(CurrentImageIndex == -1);
    VkSemaphore Semaphore = RenderingComplete->GetHandle();
    VkPresentInfoKHR Info{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &Semaphore,
        .swapchainCount = 1,
        .pSwapchains = &SwapChain,
        .pImageIndices = (uint32 *)&CurrentImageIndex,
    };

    VkResult PresentResult = VulkanPlatform::Present(PresentQueue->GetHandle(), Info);

    if (PresentResult == VK_ERROR_OUT_OF_DATE_KHR) { return Status::OutOfDate; }

    if (PresentResult == VK_ERROR_SURFACE_LOST_KHR) { return Status::SurfaceLost; }

    if (PresentResult != VK_SUCCESS && PresentResult != VK_SUBOPTIMAL_KHR) { VK_CHECK_RESULT(PresentResult); }

    return Status::Healty;
}

int32 VulkanSwapChain::AcquireImageIndex(Ref<Semaphore> &OutSemaphore)
{
    check(CurrentImageIndex == -1);

    uint32 ImageIndex = 0;
    const int32 PrevSemaphoreIndex = SemaphoreIndex;
    SemaphoreIndex = (SemaphoreIndex + 1) % ImageAcquiredSemaphore.size();

    Ref<Fence> AcquiredFence = ImageInUseFence[SemaphoreIndex];
    AcquiredFence.Reset();

    VkResult Result = VulkanAPI::vkAcquireNextImageKHR(Device->GetInstanceHandle(), SwapChain, UINT64_MAX,
                                                       ImageAcquiredSemaphore[SemaphoreIndex]->GetHandle(),
                                                       AcquiredFence->GetHandle(), &ImageIndex);

    if (Result == VK_ERROR_OUT_OF_DATE_KHR) {
        SemaphoreIndex = PrevSemaphoreIndex;
        return (int32)Status::OutOfDate;
    }
    if (Result == VK_ERROR_SURFACE_LOST_KHR) {
        SemaphoreIndex = PrevSemaphoreIndex;
        return (int32)Status::SurfaceLost;
    }
    OutSemaphore = ImageAcquiredSemaphore[SemaphoreIndex];

    checkMsg(Result == VK_SUCCESS || Result == VK_SUBOPTIMAL_KHR, "vkAcquireNextImageKHR failed Result = {}",
             int32(Result));
    CurrentImageIndex = (int32)ImageIndex;

    verify(ImageInUseFence[SemaphoreIndex]->Wait(UINT64_MAX));

    return CurrentImageIndex;
}

}    // namespace VulkanRHI

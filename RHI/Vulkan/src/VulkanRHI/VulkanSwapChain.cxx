#include "VulkanRHI/VulkanSwapChain.hxx"

#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanLoader.hxx"
#include "VulkanRHI/VulkanPlatform.hxx"
#include "VulkanRHI/VulkanQueue.hxx"
#include "VulkanRHI/VulkanSynchronization.hxx"
#include "VulkanRHI/VulkanUtils.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogVulkanSwapchain, Info);

namespace VulkanRHI
{

RVulkanSwapChain::FSupportDetails RVulkanSwapChain::FSupportDetails::QuerySwapChainSupport(const FVulkanDevice* Device,
                                                                                           const VkSurfaceKHR& Surface)
{
    RVulkanSwapChain::FSupportDetails Details;

    {
        VK_CHECK_RESULT(VulkanAPI::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device->GetPhysicalHandle(), Surface,
                                                                             &Details.Capabilities));
    }

    {
        uint32 NumFormats;
        VK_CHECK_RESULT(VulkanAPI::vkGetPhysicalDeviceSurfaceFormatsKHR(Device->GetPhysicalHandle(), Surface,
                                                                        &NumFormats, nullptr));
        check(NumFormats > 0);

        Details.Formats.Resize(NumFormats);
        VK_CHECK_RESULT(VulkanAPI::vkGetPhysicalDeviceSurfaceFormatsKHR(Device->GetPhysicalHandle(), Surface,
                                                                        &NumFormats, Details.Formats.Raw()));
    }

    {
        uint32 NumFormats;
        VK_CHECK_RESULT(VulkanAPI::vkGetPhysicalDeviceSurfacePresentModesKHR(Device->GetPhysicalHandle(), Surface,
                                                                             &NumFormats, nullptr));
        check(NumFormats > 0);

        Details.PresentModes.Resize(NumFormats);
        VK_CHECK_RESULT(VulkanAPI::vkGetPhysicalDeviceSurfacePresentModesKHR(Device->GetPhysicalHandle(), Surface,
                                                                             &NumFormats, Details.PresentModes.Raw()));
    }
    return Details;
}

VkSurfaceFormatKHR RVulkanSwapChain::FSupportDetails::ChooseSwapSurfaceFormat() const noexcept
{
    for (const auto& availableFormat: Formats)
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;
    return Formats[0];
}

VkPresentModeKHR RVulkanSwapChain::FSupportDetails::ChooseSwapPresentMode(bool LockToVSync) const noexcept
{
    bool bFoundPresentModeMailbox = false;
    bool bFoundPresentModeImmediate = false;
    bool bFoundPresentModeFIFO = false;

    LOG(LogVulkanSwapchain, Info, "Found {} Surface present modes:", PresentModes.Size());

    for (const auto& availablePresentMode: PresentModes)
    {
        const auto Index = magic_enum::enum_index(availablePresentMode);
        LOG(LogVulkanSwapchain, Info, "- {}({})", magic_enum::enum_name(availablePresentMode),
            (Index.has_value() ? Index.value() : NAN));
        switch (availablePresentMode)
        {
            case VK_PRESENT_MODE_MAILBOX_KHR:
                bFoundPresentModeMailbox = true;
                break;
            case VK_PRESENT_MODE_IMMEDIATE_KHR:
                bFoundPresentModeImmediate = true;
                break;
            case VK_PRESENT_MODE_FIFO_KHR:
                bFoundPresentModeFIFO = true;
                break;
            default:
                break;
        }
    }

    if (bFoundPresentModeImmediate && !LockToVSync)
    {
        return VK_PRESENT_MODE_IMMEDIATE_KHR;
    }
    else if (bFoundPresentModeFIFO)
    {
        return VK_PRESENT_MODE_FIFO_KHR;
    }
    else if (bFoundPresentModeMailbox)
    {
        return VK_PRESENT_MODE_MAILBOX_KHR;
    }
    else
    {
        LOG(LogVulkanSwapchain, Warning, "Couldn't find desired PresentMode! Using {}",
            VK_TYPE_TO_STRING(VkPresentModeKHR, PresentModes[0]));
        return PresentModes[0];
    }
}

VkExtent2D RVulkanSwapChain::FSupportDetails::ChooseSwapExtent(const UVector2& InSize) const noexcept
{
    return {
        .width = std::clamp(InSize.x, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width),
        .height = std::clamp(InSize.y, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height),
    };
}

RVulkanSwapChain::RVulkanSwapChain(VkInstance InInstance, FVulkanDevice* InDevice, const UVector2& InSize,
                                   RWindow* WindowHandle, uint32 InDesiredNumBackBuffers, TArray<VkImage>& OutImages,
                                   bool LockToVSync, VulkanSwapChainRecreateInfo* RecreateInfo)
    : IDeviceChild(InDevice)
    , CurrentImageIndex(-1)
    , SemaphoreIndex(0)
    , LockToVSync(LockToVSync)
    , SwapChain(VK_NULL_HANDLE)
    , Surface(VK_NULL_HANDLE)
    , Instance(InInstance)
{
    if (RecreateInfo && RecreateInfo->SwapChain != VK_NULL_HANDLE)
    {
        check(RecreateInfo->Surface != VK_NULL_HANDLE);
        Surface = RecreateInfo->Surface;
        RecreateInfo->Surface = VK_NULL_HANDLE;
    }
    else
    {
        GetVulkanDynamicRHI()->GetVulkanPlatform().CreateSurface(WindowHandle, Instance, &Surface);
    }

    const FSupportDetails SwapChainSupport = FSupportDetails::QuerySwapChainSupport(Device, Surface);
    VkSurfaceFormatKHR SurfaceFormat = SwapChainSupport.ChooseSwapSurfaceFormat();
    VkPresentModeKHR PresentMode = SwapChainSupport.ChooseSwapPresentMode(LockToVSync);
    VkExtent2D Extent = SwapChainSupport.ChooseSwapExtent(InSize);

    uint32 ImageCount = std::max(SwapChainSupport.Capabilities.minImageCount + 1, InDesiredNumBackBuffers);
    if (SwapChainSupport.Capabilities.maxImageCount > 0 && ImageCount > SwapChainSupport.Capabilities.maxImageCount)
    {
        ImageCount = SwapChainSupport.Capabilities.maxImageCount;
    }

    Device->SetupPresentQueue(Surface);

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
        VulkanAPI::vkCreateSwapchainKHR(Device->GetHandle(), &CreateInfo, VULKAN_CPU_ALLOCATOR, &SwapChain));

    ImageFormat = SurfaceFormat.format;
    if (RecreateInfo)
    {
        if (RecreateInfo->SwapChain != VK_NULL_HANDLE)
        {
            RHI::DeferedDeletion(
                [Device = this->Device, SwapChain = RecreateInfo->SwapChain]
                { VulkanAPI::vkDestroySwapchainKHR(Device->GetHandle(), SwapChain, VULKAN_CPU_ALLOCATOR); });
            RecreateInfo->SwapChain = VK_NULL_HANDLE;
        }
        if (RecreateInfo->Surface != VK_NULL_HANDLE)
        {
            RHI::DeferedDeletion([Instance = this->Instance, Surface = RecreateInfo->Surface]
                                 { VulkanAPI::vkDestroySurfaceKHR(Instance, Surface, VULKAN_CPU_ALLOCATOR); });
            RecreateInfo->Surface = VK_NULL_HANDLE;
        }
    }

    InternalSize.x = CreateInfo.imageExtent.width;
    InternalSize.y = CreateInfo.imageExtent.height;

    uint32 NumSwapchainImages;
    VK_CHECK_RESULT_EXPANDED(
        VulkanAPI::vkGetSwapchainImagesKHR(Device->GetHandle(), SwapChain, &NumSwapchainImages, nullptr));
    OutImages.Resize(NumSwapchainImages);
    VK_CHECK_RESULT_EXPANDED(
        VulkanAPI::vkGetSwapchainImagesKHR(Device->GetHandle(), SwapChain, &NumSwapchainImages, OutImages.Raw()));

    ImageAcquiredSemaphore.Resize(NumSwapchainImages);
    for (uint32 BufferIndex = 0; BufferIndex < NumSwapchainImages; BufferIndex++)
    {
        ImageAcquiredSemaphore[BufferIndex] = Ref<RSemaphore>::Create(Device);
        ImageAcquiredSemaphore[BufferIndex]->SetName(std::format("{}.SwapchainImageAcquired", BufferIndex));
    }

    ImageInUseFence.Resize(NumSwapchainImages);
    for (uint32 BufferIndex = 0; BufferIndex < NumSwapchainImages; BufferIndex++)
    {
        ImageInUseFence[BufferIndex] = Ref<RFence>::Create(Device, true);
        ImageInUseFence[BufferIndex]->SetName(std::format("{}.SwapchainImageInUse", BufferIndex));
    }
}

void RVulkanSwapChain::Destroy(VulkanSwapChainRecreateInfo* RecreateInfo)
{
    if (RecreateInfo)
    {
        RecreateInfo->SwapChain = SwapChain;
        RecreateInfo->Surface = Surface;
    }
    else
    {
        RHI::DeferedDeletion(
            [Device = this->Device, SwapChain = this->SwapChain]
            { VulkanAPI::vkDestroySwapchainKHR(Device->GetHandle(), SwapChain, VULKAN_CPU_ALLOCATOR); });
        RHI::DeferedDeletion([Instance = this->Instance, Surface = this->Surface]
                             { VulkanAPI::vkDestroySurfaceKHR(Instance, Surface, VULKAN_CPU_ALLOCATOR); });
    }
    Surface = VK_NULL_HANDLE;
    SwapChain = VK_NULL_HANDLE;
    ImageAcquiredSemaphore.Clear();
    ImageInUseFence.Clear();
}

RVulkanSwapChain::EStatus RVulkanSwapChain::Present(FVulkanQueue* PresentQueue, Ref<RSemaphore>& RenderingComplete)
{
    RPH_PROFILE_FUNC()

    check(CurrentImageIndex != -1);

    VkSemaphore Semaphore = VK_NULL_HANDLE;
    VkPresentInfoKHR Info{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .swapchainCount = 1,
        .pSwapchains = &SwapChain,
        .pImageIndices = (uint32*)&CurrentImageIndex,
    };
    if (RenderingComplete)
    {
        Semaphore = RenderingComplete->GetHandle();
        Info.waitSemaphoreCount = 1;
        Info.pWaitSemaphores = &Semaphore;
    }

    VkResult PresentResult = VulkanAPI::vkQueuePresentKHR(PresentQueue->GetHandle(), &Info);

    CurrentImageIndex = -1;

    if (PresentResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        return EStatus::OutOfDate;
    }

    if (PresentResult == VK_ERROR_SURFACE_LOST_KHR)
    {
        return EStatus::SurfaceLost;
    }

    if (PresentResult != VK_SUCCESS && PresentResult != VK_SUBOPTIMAL_KHR)
    {
        VK_CHECK_RESULT(PresentResult);
    }

    return EStatus::Healty;
}

void RVulkanSwapChain::SetName(std::string_view InName)
{
    Super::SetName(InName);
    VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_SWAPCHAIN_KHR, SwapChain, "{:s}", InName);
}

int32 RVulkanSwapChain::AcquireImageIndex(Ref<RSemaphore>& OutSemaphore)
{
    RPH_PROFILE_FUNC()

    check(CurrentImageIndex == -1);

    uint32 ImageIndex = 0;
    const int32 PrevSemaphoreIndex = SemaphoreIndex;
    SemaphoreIndex = (SemaphoreIndex + 1) % ImageAcquiredSemaphore.Size();

    Ref<RFence>& AcquiredFence = ImageInUseFence[SemaphoreIndex];
    AcquiredFence->Reset();

    VkResult Result = VulkanAPI::vkAcquireNextImageKHR(Device->GetHandle(), SwapChain, UINT64_MAX,
                                                       ImageAcquiredSemaphore[SemaphoreIndex]->GetHandle(),
                                                       AcquiredFence->GetHandle(), &ImageIndex);

    if (Result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        SemaphoreIndex = PrevSemaphoreIndex;
        return (int32)EStatus::OutOfDate;
    }
    if (Result == VK_ERROR_SURFACE_LOST_KHR)
    {
        SemaphoreIndex = PrevSemaphoreIndex;
        return (int32)EStatus::SurfaceLost;
    }
    OutSemaphore = ImageAcquiredSemaphore[SemaphoreIndex];

#if VULKAN_DEBUGGING_ENABLED
    if (Result != VK_ERROR_VALIDATION_FAILED_EXT)
#endif
    {
        checkMsg(Result == VK_SUCCESS || Result == VK_SUBOPTIMAL_KHR, "vkAcquireNextImageKHR failed Result = {:s}({})",
                 VK_TYPE_TO_STRING(VkResult, Result), int32(Result));
    }
    CurrentImageIndex = (int32)ImageIndex;

    ensure(ImageInUseFence[SemaphoreIndex]->Wait(UINT64_MAX));

    return CurrentImageIndex;
}

}    // namespace VulkanRHI

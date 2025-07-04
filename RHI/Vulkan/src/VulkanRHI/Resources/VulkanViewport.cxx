#include "VulkanRHI/Resources/VulkanViewport.hxx"

#include "Engine/Misc/CommandLine.hxx"

#include "VulkanRHI/VulkanCommandContext.hxx"
#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanQueue.hxx"
#include "VulkanRHI/VulkanRHI.hxx"
#include "VulkanRHI/VulkanSwapChain.hxx"
#include "VulkanRHI/VulkanSynchronization.hxx"

#include "Engine/Core/Window.hxx"
#include "Engine/UI/Slate.hxx"

namespace VulkanRHI
{

RVulkanViewport::RVulkanViewport(FVulkanDevice* InDevice, Ref<RWindow> InWindowHandle, UVector2 InSize,
                                 bool bCreateDepthBuffer)
    : IDeviceChild(InDevice)
    , bCreateDepthBuffer(bCreateDepthBuffer)
    , WindowHandle(InWindowHandle)
    , Size(InSize)
    , AcquiredImageIndex(-1)
    , AcquiredSemaphore(nullptr)
{
    CreateSwapchain(nullptr);
}

RVulkanViewport::~RVulkanViewport()
{
    DeleteSwapchain(nullptr);

    for (VulkanTextureView& View: TexturesViews)
    {
        View.Destroy(Device);
    }
    TexturesViews.Clear();
    RenderingDoneSemaphores.Clear();

    SlateInstance = nullptr;
}

void RVulkanViewport::SetName(std::string_view InName)
{
    Super::SetName(InName);

    if (!SwapChain)
        return;

    SwapChain->SetName(InName);
    RenderingBackbuffer->SetName(std::format("{:s}.BackBuffer", InName));

    check(BackBufferImages.Size() == RenderingDoneSemaphores.Size());
    check(BackBufferImages.Size() == TexturesViews.Size());

    for (unsigned i = 0; i < BackBufferImages.Size(); i++)
    {
        RenderingDoneSemaphores[i]->SetName(std::format("{:s}.RenderingDone{:d}", InName, i));

        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_IMAGE, TexturesViews[i].Image, "{:s}.Image{:d}", InName, i);
        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_IMAGE_VIEW, TexturesViews[i].View, "{:s}.Image{:d}.View", InName,
                              i);
    }
}

void RVulkanViewport::ResizeViewport(uint32_t Width, uint32_t Height)
{
    Size = {Width, Height};
    // Just recreate the swapchain, the backbuffer will be reset as well
    RecreateSwapchain(WindowHandle);
}

static void CopyImageToBackBuffer(FVulkanCmdBuffer* CmdBuffer, RVulkanTexture* SrcSurface, VkImage DstSurface,
                                  UVector2 Size, UVector2 WindowSize)
{

    const VkImageLayout OldLayout = SrcSurface->GetLayout();
    SrcSurface->SetLayout(CmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    const VkImageSubresourceRange Range = FBarrier::MakeSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1);
    VulkanSetImageLayout(CmdBuffer->GetHandle(), DstSurface, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, Range);

    if (Size != WindowSize)
    {
        VkImageBlit Region;
        std::memset(&Region, 0, sizeof(Region));
        Region.srcOffsets[0].x = 0;
        Region.srcOffsets[0].y = 0;
        Region.srcOffsets[0].z = 0;
        Region.srcOffsets[1].x = Size.x;
        Region.srcOffsets[1].y = Size.y;
        Region.srcOffsets[1].z = 1;
        Region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        Region.srcSubresource.layerCount = 1;
        Region.dstOffsets[0].x = 0;
        Region.dstOffsets[0].y = 0;
        Region.dstOffsets[0].z = 0;
        Region.dstOffsets[1].x = WindowSize.x;
        Region.dstOffsets[1].y = WindowSize.y;
        Region.dstOffsets[1].z = 1;
        Region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        Region.dstSubresource.baseArrayLayer = 0;
        Region.dstSubresource.layerCount = 1;
        VulkanAPI::vkCmdBlitImage(CmdBuffer->GetHandle(), SrcSurface->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                  DstSurface, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Region, VK_FILTER_LINEAR);
    }
    else
    {
        VkImageCopy Region;
        std::memset(&Region, 0, sizeof(Region));
        Region.extent.width = Size.x;
        Region.extent.height = Size.y;
        Region.extent.depth = 1;
        Region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        Region.srcSubresource.layerCount = 1;
        Region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        Region.dstSubresource.layerCount = 1;
        VulkanAPI::vkCmdCopyImage(CmdBuffer->GetHandle(), SrcSurface->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                  DstSurface, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Region);
    }

    VulkanSetImageLayout(CmdBuffer->GetHandle(), DstSurface, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, Range);
    SrcSurface->SetLayout(CmdBuffer, OldLayout);
}

bool RVulkanViewport::Present(FVulkanCommandContext* Context, FVulkanCmdBuffer* CmdBuffer, FVulkanQueue* Queue,
                              FVulkanQueue* PresentQueue)
{
    check(CmdBuffer->IsOutsideRenderPass());

    if (TryAcquireImageIndex()) [[likely]]
    {
        CopyImageToBackBuffer(CmdBuffer, RenderingBackbuffer.Raw(), BackBufferImages[AcquiredImageIndex], Size,
                              SwapChain->GetInternalSize());
    }

    CmdBuffer->End();
    if (AcquiredImageIndex != -1) [[likely]]
    {
        check(AcquiredSemaphore);
        CmdBuffer->AddWaitSemaphore(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, AcquiredSemaphore);
        Ref<RSemaphore> SignalSemaphore =
            (AcquiredImageIndex >= 0) ? RenderingDoneSemaphores[AcquiredImageIndex] : nullptr;
        Context->GetCommandManager()->SubmitActiveCmdBufferFromPresent(SignalSemaphore);
    }
    else [[unlikely]]
    {
        LOG(LogVulkanRHI, Info, "AcquireNextImage() failed due to outdated swapchain, recreating");
        Queue->Submit(CmdBuffer);
        Device->WaitUntilIdle();
        RecreateSwapchain(WindowHandle);
        return true;
    }

    bool bResult = TryPresenting(PresentQueue);

    AcquiredImageIndex = -1;

    return bResult;
}

void RVulkanViewport::RecreateSwapchain(Ref<RWindow> NewNativeWindow)
{
    RHI::RHIWaitUntilIdle();

    ENQUEUE_RENDER_COMMAND(RecreateSwapchainCommand)
    (
        [this, NewNativeWindow](FFRHICommandList&)
        {
            VulkanSwapChainRecreateInfo RecreateInfo = {VK_NULL_HANDLE, VK_NULL_HANDLE};
            DeleteSwapchain(&RecreateInfo);
            WindowHandle = NewNativeWindow;
            CreateSwapchain(&RecreateInfo);
            check(RecreateInfo.Surface == VK_NULL_HANDLE);
            check(RecreateInfo.SwapChain == VK_NULL_HANDLE);
        });
}

void RVulkanViewport::CreateSwapchain(VulkanSwapChainRecreateInfo* RecreateInfo)
{
    FVulkanDynamicRHI* RHI = GetVulkanDynamicRHI();

    RenderingBackbuffer = nullptr;
    bool bLocktoVSync = FCommandLine::Param("-vsync");
    SwapChain = Ref<RVulkanSwapChain>::Create(RHI->GetInstance(), Device, Size, WindowHandle.Raw(), 3, BackBufferImages,
                                              bLocktoVSync, RecreateInfo);

    RenderingDoneSemaphores.Resize(BackBufferImages.Size());
    for (unsigned i = 0; i < BackBufferImages.Size(); i++)
    {
        RenderingDoneSemaphores[i] = Ref<RSemaphore>::Create(Device);
    }

    FVulkanCmdBuffer* CmdBuffer = Device->GetImmediateContext()->GetCommandManager()->GetUploadCmdBuffer();
    check(CmdBuffer);
    ensureAlways(CmdBuffer->IsOutsideRenderPass());

    VkClearColorValue ClearColor;
    std::memset(&ClearColor, 0, sizeof(VkClearColorValue));

    const VkImageSubresourceRange Range = FBarrier::MakeSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

    TexturesViews.Resize(BackBufferImages.Size());
    for (unsigned i = 0; i < BackBufferImages.Size(); i++)
    {
        TexturesViews[i].Create(Device, BackBufferImages[i], VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT,
                                SwapChain->GetFormat(), 0, 1);
        VulkanSetImageLayout(CmdBuffer->GetHandle(), BackBufferImages[i], VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, Range);
        VulkanAPI::vkCmdClearColorImage(CmdBuffer->GetHandle(), BackBufferImages[i],
                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &ClearColor, 1, &Range);
        VulkanSetImageLayout(CmdBuffer->GetHandle(), BackBufferImages[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, Range);
    }

    if (!RenderingBackbuffer)
    {
        Size = SwapChain->GetInternalSize();
        FRHITextureSpecification Description{
            .Flags = ETextureUsageFlags::RenderTargetable | ETextureUsageFlags::ResolveTargetable |
                     ETextureUsageFlags::TransferTargetable,
            .Dimension = EImageDimension::Texture2D,
            .Format = VkFormatToImageFormat(SwapChain->GetFormat()),
            .Extent = Size,
            .Name = std::format("{:s}.BackBuffer", GetName()),
        };
        RenderingBackbuffer = RHI::CreateTexture(Description);

        if (bCreateDepthBuffer)
        {
            FRHITextureSpecification DepthTexture = Description;
            DepthTexture.Format = EImageFormat::D32_SFLOAT;
            DepthTexture.Flags = ETextureUsageFlags::DepthStencilTargetable;
            DepthBuffer = RHI::CreateTexture(DepthTexture);
        }
    }
    RenderingBackbuffer->SetLayout(CmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VulkanAPI::vkCmdClearColorImage(CmdBuffer->GetHandle(), RenderingBackbuffer->GetImage(),
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &ClearColor, 1, &Range);
    RenderingBackbuffer->SetLayout(CmdBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    Device->GetImmediateContext()->GetCommandManager()->SubmitUploadCmdBuffer();
    RHI::RHIWaitUntilIdle();

    AcquiredImageIndex = -1;
}

void RVulkanViewport::DeleteSwapchain(VulkanSwapChainRecreateInfo* RecreateInfo)
{
    RHI::RHIWaitUntilIdle();

    for (unsigned Index = 0; Index < BackBufferImages.Size(); Index++)
    {
        TexturesViews[Index].Destroy(Device);
        BackBufferImages[Index] = VK_NULL_HANDLE;
    }

    SwapChain->Destroy(RecreateInfo);
    SwapChain = nullptr;

    AcquiredImageIndex = -1;
}

bool RVulkanViewport::TryAcquireImageIndex()
{
    if (SwapChain)
    {
        int32 Result = SwapChain->AcquireImageIndex(AcquiredSemaphore);
        if (Result >= 0 && AcquiredSemaphore)
        {
            AcquiredImageIndex = Result;
            return true;
        }
    }
    return false;
}

bool RVulkanViewport::TryPresenting(FVulkanQueue* PresentQueue)
{
    int32 AttemptsPending = 4;
    RVulkanSwapChain::EStatus Status = SwapChain->Present(PresentQueue, RenderingDoneSemaphores[AcquiredImageIndex]);

    while (Status < RVulkanSwapChain::EStatus::Healty && AttemptsPending > 0)
    {
        if (Status == RVulkanSwapChain::EStatus::OutOfDate)
        {
            LOG(LogVulkanRHI, Info, "Swapchain is out of date! Trying to recreate the swapchain.");
        }
        else if (Status == RVulkanSwapChain::EStatus::SurfaceLost)
        {
            LOG(LogVulkanRHI, Warning, "Swapchain surface lost! Trying to recreate the swapchain.");
        }
        else
        {
            checkNoEntry();
        }

        RHI::RHIWaitUntilIdle();
        RecreateSwapchain(WindowHandle);

        if (AcquiredImageIndex == -1)
        {
            return true;
        }
        Status = SwapChain->Present(PresentQueue, RenderingDoneSemaphores[AcquiredImageIndex]);

        AttemptsPending -= 1;
    }
    return Status >= RVulkanSwapChain::EStatus::Healty;
}

Ref<RSlate> RVulkanViewport::GetSlateInstance(bool bCreate)
{
    if (bCreate && !SlateInstance)
    {
        SlateInstance = Ref<RSlate>::Create(this);
    }
    return SlateInstance;
}

}    // namespace VulkanRHI

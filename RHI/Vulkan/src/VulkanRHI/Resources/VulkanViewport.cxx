#include "VulkanRHI/Resources/VulkanViewport.hxx"

#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanQueue.hxx"
#include "VulkanRHI/VulkanRHI.hxx"
#include "VulkanRHI/VulkanSwapChain.hxx"
#include "VulkanRHI/VulkanSynchronization.hxx"

namespace VulkanRHI
{

VulkanViewport::VulkanViewport(VulkanDevice* InDevice, void* InWindowHandle, glm::uvec2 InSize)
    : Device(InDevice), WindowHandle(InWindowHandle), Size(InSize), AcquiredImageIndex(-1)
{
    CreateSwapchain(nullptr);
}

VulkanViewport::~VulkanViewport()
{
    DeleteSwapchain(nullptr);

    for (VulkanTextureView& View: TexturesViews) {
        View.Destroy(Device);
    }
    TexturesViews.Clear();
    RenderingDoneSemaphores.Clear();
}

void VulkanViewport::RT_BeginDrawViewport()
{
    GetVulkanDynamicRHI()->RT_SetDrawingViewport(this);
}
void VulkanViewport::RT_EndDrawViewport()
{
    VulkanCmdBuffer* CmdBuffer = Device->GetCommandManager()->GetActiveCmdBuffer();
    check(!CmdBuffer->HasEnded() && !CmdBuffer->IsInsideRenderPass());

    this->Present(CmdBuffer, Device->GraphicsQueue.get(), Device->PresentQueue);
    GetVulkanDynamicRHI()->RT_SetDrawingViewport(nullptr);
}

void VulkanViewport::RT_ResizeViewport(uint32 Width, uint32 Height)
{
    Size.x = Width;
    Size.y = Height;
}

void VulkanViewport::SetName(std::string_view InName)
{
    RHIResource::SetName(InName);

    if (!SwapChain)
        return;

    SwapChain->SetName(InName);

    check(BackBufferImages.Size() == RenderingDoneSemaphores.Size());
    check(BackBufferImages.Size() == TexturesViews.Size());

    for (unsigned i = 0; i < BackBufferImages.Size(); i++) {
        RenderingDoneSemaphores[i]->SetName(std::format("{:s}.RenderingDone{:d}", InName, i));

        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_IMAGE, TexturesViews[i].Image, "{:s}.Image{:d}", InName, i);
        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_IMAGE_VIEW, TexturesViews[i].View, "{:s}.Image{:d}.View", InName,
                              i);
    }
}

bool VulkanViewport::Present(VulkanCmdBuffer* CmdBuffer, VulkanQueue* Queue, VulkanQueue* PresentQueue)
{
    check(CmdBuffer->IsOutsideRenderPass());

    bool bSuccesfullyAquiredImage = TryAcquireImageIndex();

    VulkanSetImageLayout(CmdBuffer->GetHandle(), BackBufferImages[AcquiredImageIndex], VK_IMAGE_LAYOUT_UNDEFINED,
                         VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, Barrier::MakeSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT));

    CmdBuffer->End();
    if (bSuccesfullyAquiredImage) [[likely]] {
        check(AcquiredSemaphore);
        CmdBuffer->AddWaitSemaphore(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, AcquiredSemaphore);
        Ref<Semaphore> SignalSemaphore =
            (AcquiredImageIndex >= 0) ? RenderingDoneSemaphores[AcquiredImageIndex] : nullptr;
        Device->GetCommandManager()->SubmitActiveCmdBufferFormPresent(SignalSemaphore);
    } else [[unlikely]] {
        LOG(LogVulkanRHI, Trace, "AcquireNextImage() failed due to outdated swapchain, recreating");
        Queue->Submit(CmdBuffer);
        RecreateSwapchain(WindowHandle);

        Device->WaitUntilIdle();
        return true;
    }

    bool bResult = TryPresenting(PresentQueue);

    AcquiredImageIndex = -1;

    return bResult;
}

void VulkanViewport::RecreateSwapchain(void* NewNativeWindow)
{
    VulkanSwapChainRecreateInfo RecreateInfo = {VK_NULL_HANDLE, VK_NULL_HANDLE};
    DeleteSwapchain(&RecreateInfo);
    WindowHandle = NewNativeWindow;
    CreateSwapchain(&RecreateInfo);
    check(RecreateInfo.Surface == VK_NULL_HANDLE);
    check(RecreateInfo.SwapChain == VK_NULL_HANDLE);
}

void VulkanViewport::CreateSwapchain(VulkanSwapChainRecreateInfo* RecreateInfo)
{
    VulkanDynamicRHI* RHI = GetVulkanDynamicRHI();

    SwapChain = Ref<VulkanSwapChain>::Create(RHI->GetInstance(), Device, WindowHandle, Size, 0, BackBufferImages, true,
                                             RecreateInfo);

    RenderingDoneSemaphores.Resize(BackBufferImages.Size());
    for (unsigned i = 0; i < BackBufferImages.Size(); i++) {
        RenderingDoneSemaphores[i] = Ref<Semaphore>::Create(Device);
    }

    VulkanCmdBuffer* CmdBuffer = Device->GetCommandManager()->GetUploadCmdBuffer();
    check(CmdBuffer);
    ensureAlways(CmdBuffer->IsOutsideRenderPass());

    VkClearColorValue ClearColor;
    std::memset(&ClearColor, 0, sizeof(VkClearColorValue));

    const VkImageSubresourceRange Range = Barrier::MakeSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

    TexturesViews.Resize(BackBufferImages.Size());
    for (unsigned i = 0; i < BackBufferImages.Size(); i++) {
        TexturesViews[i].Create(Device, BackBufferImages[i], VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT,
                                SwapChain->GetFormat(), 0, 1);
        VulkanSetImageLayout(CmdBuffer->GetHandle(), BackBufferImages[i], VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, Range);
        VulkanAPI::vkCmdClearColorImage(CmdBuffer->GetHandle(), BackBufferImages[i],
                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &ClearColor, 1, &Range);
    }

    Device->GetCommandManager()->SubmitUploadCmdBuffer();
    Device->WaitUntilIdle();

    // Refresh vulkan object name
    VulkanViewport::SetName(GetName());
}

void VulkanViewport::DeleteSwapchain(VulkanSwapChainRecreateInfo* RecreateInfo)
{
    Device->WaitUntilIdle();

    for (unsigned Index = 0; Index < BackBufferImages.Size(); Index++) {
        TexturesViews[Index].Destroy(Device);
        BackBufferImages[Index] = VK_NULL_HANDLE;
    }

    SwapChain->Destroy(RecreateInfo);
    SwapChain = nullptr;

    AcquiredImageIndex = -1;
}

bool VulkanViewport::TryAcquireImageIndex()
{
    if (SwapChain) {
        int32 Result = SwapChain->AcquireImageIndex(AcquiredSemaphore);
        if (Result >= 0) {
            AcquiredImageIndex = Result;
            return true;
        }
    }
    return false;
}

bool VulkanViewport::TryPresenting(VulkanQueue* PresentQueue)
{
    int32 AttemptsPending = 4;
    VulkanSwapChain::Status Status = SwapChain->Present(PresentQueue, RenderingDoneSemaphores[AcquiredImageIndex]);

    while (Status < VulkanSwapChain::Status::Healty && AttemptsPending > 0) {
        if (Status == VulkanSwapChain::Status::OutOfDate) {
            LOG(LogVulkanRHI, Info, "Swapchain is out of date! Trying to recreate the swapchain.");
        } else if (Status == VulkanSwapChain::Status::SurfaceLost) {
            LOG(LogVulkanRHI, Warning, "Swapchain surface lost! Trying to recreate the swapchain.");
        } else {
            checkNoEntry();
        }

        RecreateSwapchain(WindowHandle);

        Device->WaitUntilIdle();

        if (AcquiredImageIndex == -1) {
            return true;
        }
        Status = SwapChain->Present(PresentQueue, RenderingDoneSemaphores[AcquiredImageIndex]);

        AttemptsPending -= 1;
    }
    return Status >= VulkanSwapChain::Status::Healty;
}

}    // namespace VulkanRHI

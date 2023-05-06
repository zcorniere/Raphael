#include "VulkanRHI/Resources/VulkanViewport.hxx"

#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanQueue.hxx"
#include "VulkanRHI/VulkanRHI.hxx"
#include "VulkanRHI/VulkanSwapChain.hxx"
#include "VulkanRHI/VulkanSynchronization.hxx"

namespace VulkanRHI
{

VulkanViewport::VulkanViewport(Ref<VulkanDevice> InDevice, void *InWindowHandle, glm::uvec2 InSize)
    : Device(InDevice), WindowHandle(InWindowHandle), Size(InSize), AcquiredImageIndex(-1)
{
    CreateSwapchain(nullptr);
}

VulkanViewport::~VulkanViewport()
{
    for (VulkanTextureView &View: TexturesViews) { View.Destroy(Device); }
    TexturesViews.clear();
    RenderingDoneSemaphores.clear();

    DeleteSwapchain(nullptr);
}

void VulkanViewport::SetName(std::string_view InName)
{
    RHIResource::SetName(InName);

    if (!SwapChain) return;

    SwapChain->SetName(InName);

    check(BackBufferImages.size() == RenderingDoneSemaphores.size());
    check(BackBufferImages.size() == TexturesViews.size());

    for (unsigned i = 0; i < BackBufferImages.size(); i++) {
        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_SEMAPHORE, RenderingDoneSemaphores.at(i)->GetHandle(),
                              "Swapchain \"{:s}\" - Semaphore Rendering done({})", InName, i);
        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_IMAGE, TexturesViews.at(i).Image,
                              "Swapchain \"{:s}\" - Image ({})", InName, i);
        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_IMAGE_VIEW, TexturesViews.at(i).View,
                              "Swapchain \"{:s}\" - Image View ({})", InName, i);
    }
}

bool VulkanViewport::Present(Ref<VulkanCmdBuffer> &CmdBuffer, Ref<VulkanQueue> &Queue, Ref<VulkanQueue> &PresentQueue)
{
    check(CmdBuffer->IsOutsideRenderPass());

    bool bSuccesfullyAquiredImage = TryAcquireImageIndex();

    CmdBuffer->End();
    if (LIKELY(bSuccesfullyAquiredImage)) {
        Ref<Semaphore> SignalSemaphore =
            (AcquiredImageIndex >= 0) ? RenderingDoneSemaphores[AcquiredImageIndex] : nullptr;
        Device->GetCommandManager()->SubmitActiveCmdBufferFormPresent(SignalSemaphore);
    } else {
        LOG(LogVulkanRHI, Trace, "AcquireNextImage() failed due to outdated swapchain, recreating");
        Queue->Submit(CmdBuffer);
        RecreateSwapchain(WindowHandle);

        Device->WaitUntilIdle();

        return true;
    }

    bool bResult = false;
    if (SwapChain->Present(PresentQueue, RenderingDoneSemaphores[AcquiredImageIndex]) !=
        VulkanSwapChain::Status::Healty) {
        LOG(LogVulkanRHI, Error, "Swapchain present failed !");
        bResult = false;
    } else {
        bResult = true;
    }

    AcquiredImageIndex = -1;

    return bResult;
}

void VulkanViewport::RecreateSwapchain(void *NewNativeWindow)
{
    VulkanSwapChainRecreateInfo RecreateInfo = {VK_NULL_HANDLE, VK_NULL_HANDLE};
    DeleteSwapchain(&RecreateInfo);
    WindowHandle = NewNativeWindow;
    CreateSwapchain(&RecreateInfo);
    check(RecreateInfo.Surface == VK_NULL_HANDLE);
    check(RecreateInfo.SwapChain == VK_NULL_HANDLE);
}

void VulkanViewport::CreateSwapchain(VulkanSwapChainRecreateInfo *RecreateInfo)
{
    Ref<VulkanDynamicRHI> RHI = GetVulkanDynamicRHI();

    SwapChain = Ref<VulkanSwapChain>::Create(RHI->RHIGetVkInstance(), Device, WindowHandle, Size, 0, BackBufferImages,
                                             true, RecreateInfo);

    RenderingDoneSemaphores.resize(BackBufferImages.size());
    for (unsigned i = 0; i < BackBufferImages.size(); i++) {
        RenderingDoneSemaphores[i] = Ref<Semaphore>::Create(Device);
    }

    Ref<VulkanCmdBuffer> &CmdBuffer = Device->GetCommandManager()->GetUploadCmdBuffer();
    verify(CmdBuffer->IsOutsideRenderPass());

    // VkClearColorValue ClearColor;
    // std::memset(&ClearColor, 0, sizeof(VkClearColorValue));

    // const VkImageSubresourceRange Range{
    //     .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    //     .baseMipLevel = 0,
    //     .levelCount = VK_REMAINING_MIP_LEVELS,
    //     .baseArrayLayer = 0,
    //     .layerCount = VK_REMAINING_ARRAY_LAYERS,
    // };

    TexturesViews.resize(BackBufferImages.size());
    for (unsigned i = 0; i < BackBufferImages.size(); i++) {
        TexturesViews.at(i).Create(Device, BackBufferImages.at(i), VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT,
                                   SwapChain->GetFormat(), 0, 1);
        // VulkanAPI::vkCmdClearColorImage(CmdBuffer->GetHandle(), BackBufferImages[i], VK_IMAGE_LAYOUT_UNDEFINED,
        //                                 &ClearColor, 1, &Range);
    }

    Device->GetCommandManager()->SubmitUploadCmdBuffer();

    // Refresh vulkan object name
    VulkanViewport::SetName(GetName());
}

void VulkanViewport::DeleteSwapchain(VulkanSwapChainRecreateInfo *RecreateInfo)
{
    Device->WaitUntilIdle();

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

}    // namespace VulkanRHI

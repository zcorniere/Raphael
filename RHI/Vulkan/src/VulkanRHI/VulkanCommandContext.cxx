#include "VulkanRHI/VulkanCommandContext.hxx"

#include "VulkanRHI/Resources/VulkanViewport.hxx"
#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanRHI.hxx"

namespace VulkanRHI
{

VulkanCommandContext::VulkanCommandContext(VulkanDevice* InDevice, VulkanQueue* InGraphicsQueue,
                                           VulkanQueue* InPresentQueue)
    : Device(InDevice), GfxQueue(InGraphicsQueue), PresentQueue(InPresentQueue)
{
}

VulkanCommandContext::~VulkanCommandContext()
{
}

void VulkanCommandContext::BeginFrame()
{
    Device->GetCommandManager()->PrepareForNewActiveCommandBuffer();
}

void VulkanCommandContext::EndFrame()
{
}

void VulkanCommandContext::RHIBeginDrawingViewport(RHIViewport* const Viewport)
{
    VulkanViewport* const VKViewport = dynamic_cast<VulkanViewport*>(Viewport);
    GetVulkanDynamicRHI()->DrawingViewport = VKViewport;
}

void VulkanCommandContext::RHIEndDrawningViewport(RHIViewport* const Viewport)
{
    VulkanViewport* const VKViewport = dynamic_cast<VulkanViewport*>(Viewport);
    VKViewport->Present(Device->GetCommandManager()->GetActiveCmdBuffer(), GfxQueue, PresentQueue);

    check(GetVulkanDynamicRHI()->DrawingViewport == Viewport);
    GetVulkanDynamicRHI()->DrawingViewport = nullptr;
}

void VulkanCommandContext::RHIResizeViewport(RHIViewport* const Viewport, uint32_t Width, uint32_t Height)
{
    VulkanViewport* const VKViewport = dynamic_cast<VulkanViewport*>(Viewport);

    VKViewport->SetSize({Width, Height});

    Ref<VulkanTexture> Backbuffer = Viewport->GetBackbuffer().As<VulkanTexture>();

    VkImageLayout Layout = Backbuffer->GetLayout();
    Backbuffer->Resize({Width, Height});

    SetLayout(Backbuffer.Raw(), Layout);
}

void VulkanCommandContext::SetLayout(VulkanTexture* const Texture, VkImageLayout Layout)
{
    Texture->SetLayout(Device->GetCommandManager()->GetActiveCmdBuffer(), Layout);
}

}    // namespace VulkanRHI

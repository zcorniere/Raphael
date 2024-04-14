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

void VulkanCommandContext::RHIBeginRendering(const RHIRenderPassDescription& Description)
{
    auto RenderTargetToAttachmentInfo = [](const RHIRenderTarget& Target) -> VkRenderingAttachmentInfo {
        Ref<VulkanTexture> const Texture = Target.Texture.As<VulkanTexture>();

        VkClearColorValue ClearColor;
        std::memset(&ClearColor, 0, sizeof(VkClearColorValue));
        ClearColor.uint32[0] = Target.ClearColor.r;
        ClearColor.uint32[1] = Target.ClearColor.g;
        ClearColor.uint32[2] = Target.ClearColor.b;
        ClearColor.uint32[3] = Target.ClearColor.a;

        return VkRenderingAttachmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .pNext = nullptr,
            .imageView = Texture->GetImageView(),
            .imageLayout = Texture->GetLayout(),
            .loadOp = RenderTargetLoadActionToVkAttachmentLoadOp(Target.LoadAction),
            .storeOp = RenderTargetStoreActionToVkAttachmentStoreOp(Target.StoreAction),
            .clearValue = {ClearColor},
        };
    };
    VulkanCmdBuffer* CmdBuffer = Device->GetCommandManager()->GetActiveCmdBuffer();

    Array<VkRenderingAttachmentInfo> ColorAttachments;
    ColorAttachments.Reserve(Description.ColorTargets.Size());
    for (const RHIRenderTarget& ColorTarget: Description.ColorTargets) {
        ColorAttachments.Add(RenderTargetToAttachmentInfo(ColorTarget));
    }
    std::optional<VkRenderingAttachmentInfo> DepthAttachment = std::nullopt;
    if (Description.DepthTarget) {
        DepthAttachment = RenderTargetToAttachmentInfo(Description.DepthTarget.value());
    }

    VkRenderingInfo RenderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .pNext = nullptr,
        .renderArea =
            {
                .offset = {Description.RenderAreaLocation.x, Description.RenderAreaLocation.y},
                .extent = {Description.RenderAreaSize.x, Description.RenderAreaSize.y},
            },
        .layerCount = 1,
        .colorAttachmentCount = ColorAttachments.Size(),
        .pColorAttachments = ColorAttachments.Raw(),
        .pDepthAttachment = DepthAttachment.has_value() ? &DepthAttachment.value() : nullptr,
    };
    CmdBuffer->BeginRendering(RenderingInfo);
}

void VulkanCommandContext::RHIEndRendering()
{
    VulkanCmdBuffer* CmdBuffer = Device->GetCommandManager()->GetActiveCmdBuffer();

    CmdBuffer->EndRendering();
}

void VulkanCommandContext::SetLayout(VulkanTexture* const Texture, VkImageLayout Layout)
{
    Texture->SetLayout(Device->GetCommandManager()->GetActiveCmdBuffer(), Layout);
}

}    // namespace VulkanRHI

#include "VulkanRHI/VulkanCommandContext.hxx"

#include "VulkanRHI/Resources/VulkanGraphicsPipeline.hxx"
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
        ClearColor.float32[0] = Target.ClearColor.r;
        ClearColor.float32[1] = Target.ClearColor.g;
        ClearColor.float32[2] = Target.ClearColor.b;
        ClearColor.float32[3] = Target.ClearColor.a;

        return VkRenderingAttachmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .pNext = nullptr,
            .imageView = Texture->GetImageView(),
            .imageLayout = Texture->GetLayout(),
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .loadOp = RenderTargetLoadActionToVkAttachmentLoadOp(Target.LoadAction),
            .storeOp = RenderTargetStoreActionToVkAttachmentStoreOp(Target.StoreAction),
            .clearValue = {.color = ClearColor},
        };
    };
    auto TransitionToCorrectLayout = [this](const RHIRenderTarget& Target) -> bool {
        Ref<VulkanTexture> Texture = Target.Texture.As<VulkanTexture>();
        VkImageLayout ExpectedLayout = Texture->GetDefaultLayout();
        if (ExpectedLayout != VK_IMAGE_LAYOUT_UNDEFINED && ExpectedLayout != Texture->GetLayout()) {
            Texture->SetLayout(Device->GetCommandManager()->GetUploadCmdBuffer(), ExpectedLayout);
            return true;
        }
        return false;
    };

    bool bNeedTransition = false;
    Array<VkRenderingAttachmentInfo> ColorAttachments;
    ColorAttachments.Reserve(Description.ColorTargets.Size());
    for (const RHIRenderTarget& ColorTarget: Description.ColorTargets) {
        bNeedTransition |= TransitionToCorrectLayout(ColorTarget);
        ColorAttachments.Add(RenderTargetToAttachmentInfo(ColorTarget));
    }
    std::optional<VkRenderingAttachmentInfo> DepthAttachment = std::nullopt;
    if (Description.DepthTarget) {
        bNeedTransition |= TransitionToCorrectLayout(Description.DepthTarget.value());
        DepthAttachment = RenderTargetToAttachmentInfo(Description.DepthTarget.value());
    }

    if (bNeedTransition) {
        Device->GetCommandManager()->SubmitUploadCmdBuffer();
    }
    VkRenderingInfo RenderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .pNext = nullptr,
        .flags = 0,
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

    VulkanCmdBuffer* CmdBuffer = Device->GetCommandManager()->GetActiveCmdBuffer();
    CmdBuffer->BeginRendering(RenderingInfo);
}

void VulkanCommandContext::RHIEndRendering()
{
    VulkanCmdBuffer* CmdBuffer = Device->GetCommandManager()->GetActiveCmdBuffer();

    CmdBuffer->EndRendering();
}

void VulkanCommandContext::TmpDraw(Ref<RHIGraphicsPipeline>& Pipeline)
{
    VulkanCmdBuffer* CmdBuffer = Device->GetCommandManager()->GetActiveCmdBuffer();

    Ref<VulkanGraphicsPipeline> VulkanPipeline = Pipeline.As<VulkanGraphicsPipeline>();
    VulkanPipeline->Bind(CmdBuffer->GetHandle());

    VkViewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(GetVulkanDynamicRHI()->DrawingViewport->GetSize().x),
        .height = static_cast<float>(GetVulkanDynamicRHI()->DrawingViewport->GetSize().y),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    VkRect2D scissor{
        .offset =
            {
                .x = 0,
                .y = 0,
            },
        .extent =
            {
                .width = GetVulkanDynamicRHI()->DrawingViewport->GetSize().x,
                .height = GetVulkanDynamicRHI()->DrawingViewport->GetSize().y,
            },
    };
    VulkanAPI::vkCmdSetViewport(CmdBuffer->GetHandle(), 0, 1, &viewport);
    VulkanAPI::vkCmdSetScissor(CmdBuffer->GetHandle(), 0, 1, &scissor);
    // DELETE ME
    VulkanAPI::vkCmdDraw(CmdBuffer->GetHandle(), 3, 1, 0, 0);
}

void VulkanCommandContext::SetLayout(VulkanTexture* const Texture, VkImageLayout Layout)
{
    Texture->SetLayout(Device->GetCommandManager()->GetActiveCmdBuffer(), Layout);
}

}    // namespace VulkanRHI

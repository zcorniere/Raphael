#include "VulkanRHI/VulkanCommandContext.hxx"

#include "VulkanRHI/Resources/VulkanBuffer.hxx"
#include "VulkanRHI/Resources/VulkanGraphicsPipeline.hxx"
#include "VulkanRHI/Resources/VulkanViewport.hxx"
#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanPendingState.hxx"
#include "VulkanRHI/VulkanRHI.hxx"

namespace VulkanRHI
{

VulkanCommandContext::VulkanCommandContext(VulkanDevice* InDevice, VulkanQueue* InGraphicsQueue,
                                           VulkanQueue* InPresentQueue)
    : Device(InDevice), GfxQueue(InGraphicsQueue), PresentQueue(InPresentQueue)
{
    PendingState = std::make_unique<VulkanPendingState>(Device, *this);

    // TODO: Differentiate between immediate context and frame context to use the transfer Queue
    CommandManager = std::make_unique<VulkanCommandBufferManager>(Device, GfxQueue);
}

VulkanCommandContext::~VulkanCommandContext()
{
}

void VulkanCommandContext::Reset()
{
    VulkanDevice* const ODevice = this->Device;
    VulkanQueue* const OGfxQueue = this->GfxQueue;
    VulkanQueue* const OPresentQueue = this->PresentQueue;

    // Not the best to do it, but I find it funnier to do it this way
    VulkanCommandContext::~VulkanCommandContext();
    new (this) VulkanCommandContext(ODevice, OGfxQueue, OPresentQueue);    // Placement new (reconstruct the object
}

void VulkanCommandContext::BeginFrame()
{
    CommandManager->PrepareForNewActiveCommandBuffer();
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
    VKViewport->Present(this, CommandManager->GetActiveCmdBuffer(), GfxQueue, PresentQueue);

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
            Texture->SetLayout(CommandManager->GetUploadCmdBuffer(), ExpectedLayout);
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
        CommandManager->SubmitUploadCmdBuffer();
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

    VulkanCmdBuffer* CmdBuffer = CommandManager->GetActiveCmdBuffer();
    CmdBuffer->BeginRendering(RenderingInfo);
}

void VulkanCommandContext::RHIEndRendering()
{
    VulkanCmdBuffer* CmdBuffer = CommandManager->GetActiveCmdBuffer();

    CmdBuffer->EndRendering();
}

void VulkanCommandContext::SetPipeline(Ref<RHIGraphicsPipeline>& Pipeline)
{
    Ref<VulkanGraphicsPipeline> VulkanPipeline = Pipeline.As<VulkanGraphicsPipeline>();
    PendingState->SetGraphicsPipeline(VulkanPipeline);
}

void VulkanCommandContext::SetVertexBuffer(Ref<RHIBuffer>& VertexBuffer, uint32 BufferIndex, uint32 Offset)
{
    Ref<VulkanBuffer> VulkanBufferRef = VertexBuffer.As<VulkanBuffer>();
    PendingState->SetVertexBuffer(VulkanBufferRef, BufferIndex, Offset);
}

void VulkanCommandContext::SetViewport(glm::vec3 Min, glm::vec3 Max)
{
    PendingState->SetViewport(Min, Max);
}

void VulkanCommandContext::SetScissor(glm::ivec2 Offset, glm::uvec2 Size)
{
    PendingState->SetScissor(Offset, Size);
}

void VulkanCommandContext::Draw(uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances)
{

    VulkanCmdBuffer* CmdBuffer = CommandManager->GetActiveCmdBuffer();
    PendingState->PrepareForDraw(CmdBuffer);
    VulkanAPI::vkCmdDraw(CmdBuffer->GetHandle(), BaseVertexIndex, NumPrimitives, NumInstances, 0);
}

void VulkanCommandContext::CopyBufferToBuffer(const Ref<RHIBuffer>& Source, Ref<RHIBuffer>& Destination,
                                              uint64 SourceOffset, uint64 DestinationOffset, uint64 Size)
{
    const VulkanBuffer* const SrcBuffer = Source.AsRaw<VulkanBuffer>();
    VulkanBuffer* const DstBuffer = Destination.AsRaw<VulkanBuffer>();
    const VkBufferCopy copyRegion{
        .srcOffset = SourceOffset,
        .dstOffset = DestinationOffset,
        .size = Size,
    };
    VulkanCmdBuffer* CmdBuffer = CommandManager->GetUploadCmdBuffer();
    VulkanAPI::vkCmdCopyBuffer(CmdBuffer->GetHandle(), SrcBuffer->GetHandle(), DstBuffer->GetHandle(), 1, &copyRegion);
    CommandManager->SubmitUploadCmdBuffer();
}

void VulkanCommandContext::SetLayout(VulkanTexture* const Texture, VkImageLayout Layout)
{
    Texture->SetLayout(CommandManager->GetActiveCmdBuffer(), Layout);
}

}    // namespace VulkanRHI

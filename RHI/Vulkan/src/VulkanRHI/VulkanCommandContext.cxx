#include "VulkanRHI/VulkanCommandContext.hxx"

#include "VulkanRHI/Resources/VulkanBuffer.hxx"
#include "VulkanRHI/Resources/VulkanGraphicsPipeline.hxx"
#include "VulkanRHI/Resources/VulkanMaterial.hxx"
#include "VulkanRHI/Resources/VulkanViewport.hxx"
#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanMemoryManager.hxx"
#include "VulkanRHI/VulkanPendingState.hxx"
#include "VulkanRHI/VulkanRHI.hxx"

namespace VulkanRHI
{

FVulkanCommandContext::FVulkanCommandContext(FVulkanDevice* InDevice, FVulkanQueue* InGraphicsQueue,
                                             FVulkanQueue* InPresentQueue)
    : Device(InDevice), GfxQueue(InGraphicsQueue), PresentQueue(InPresentQueue)
{
    PendingState = std::make_unique<FVulkanPendingState>(Device, *this);

    // TODO: Differentiate between immediate context and frame context to use the transfer Queue
    CommandManager = std::make_unique<VulkanCommandBufferManager>(Device, GfxQueue);
}

FVulkanCommandContext::~FVulkanCommandContext()
{
}

void FVulkanCommandContext::Reset()
{
    FVulkanDevice* const ODevice = this->Device;
    FVulkanQueue* const OGfxQueue = this->GfxQueue;
    FVulkanQueue* const OPresentQueue = this->PresentQueue;

    // Not the best to do it, but I find it funnier to do it this way
    FVulkanCommandContext::~FVulkanCommandContext();
    new (this) FVulkanCommandContext(ODevice, OGfxQueue, OPresentQueue);    // Placement new (reconstruct the object
}

void FVulkanCommandContext::BeginFrame()
{
    CommandManager->PrepareForNewActiveCommandBuffer();
}

void FVulkanCommandContext::EndFrame()
{
}

void FVulkanCommandContext::RHIBeginDrawingViewport(RRHIViewport* const Viewport)
{
    RVulkanViewport* const VKViewport = Viewport->Cast<RVulkanViewport>();
    GetVulkanDynamicRHI()->DrawingViewport = VKViewport;
}

void FVulkanCommandContext::RHIEndDrawningViewport(RRHIViewport* const Viewport)
{
    RVulkanViewport* const VKViewport = Viewport->Cast<RVulkanViewport>();
    VKViewport->Present(this, CommandManager->GetActiveCmdBuffer(), GfxQueue, PresentQueue);

    check(GetVulkanDynamicRHI()->DrawingViewport == Viewport);
    GetVulkanDynamicRHI()->DrawingViewport = nullptr;
}

void FVulkanCommandContext::RHIBeginRendering(const RHIRenderPassDescription& Description)
{
    auto RenderTargetToAttachmentInfo = [](const RHIRenderTarget& Target) -> VkRenderingAttachmentInfo {
        Ref<RVulkanTexture> const Texture = Target.Texture.As<RVulkanTexture>();

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
        Ref<RVulkanTexture> Texture = Target.Texture.As<RVulkanTexture>();
        VkImageLayout ExpectedLayout = Texture->GetDefaultLayout();
        if (ExpectedLayout != VK_IMAGE_LAYOUT_UNDEFINED && ExpectedLayout != Texture->GetLayout()) {
            Texture->SetLayout(CommandManager->GetUploadCmdBuffer(), ExpectedLayout);
            return true;
        }
        return false;
    };

    bool bNeedTransition = false;
    TArray<VkRenderingAttachmentInfo> ColorAttachments;
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

    FVulkanCmdBuffer* CmdBuffer = CommandManager->GetActiveCmdBuffer();
    CmdBuffer->BeginRendering(RenderingInfo);
}

void FVulkanCommandContext::RHIEndRendering()
{
    FVulkanCmdBuffer* CmdBuffer = CommandManager->GetActiveCmdBuffer();

    CmdBuffer->EndRendering();
}

void FVulkanCommandContext::SetPipeline(Ref<RRHIGraphicsPipeline>& PipelineState)
{
    Ref<RVulkanGraphicsPipeline> VulkanPipeline = PipelineState.As<RVulkanGraphicsPipeline>();
    PendingState->SetGraphicsPipeline(VulkanPipeline);
}

void FVulkanCommandContext::SetMaterial(Ref<RRHIMaterial>& Material)
{
    Ref<RVulkanMaterial> VulkanMaterial = Material.As<RVulkanMaterial>();
    Ref<RVulkanGraphicsPipeline> VulkanPipeline = VulkanMaterial->GetPipeline().As<RVulkanGraphicsPipeline>();
    PendingState->SetGraphicsPipeline(VulkanPipeline);
    PendingState->SetPendingDescriptorSets(VulkanMaterial->GetDescriptorSet());
}

void FVulkanCommandContext::SetVertexBuffer(Ref<RRHIBuffer>& VertexBuffer, uint32 BufferIndex, uint32 Offset)
{
    Ref<RVulkanBuffer> VulkanBufferRef = VertexBuffer.As<RVulkanBuffer>();
    PendingState->SetVertexBuffer(VulkanBufferRef, BufferIndex, Offset);
}

void FVulkanCommandContext::SetViewport(FVector3 Min, FVector3 Max)
{
    PendingState->SetViewport(Min, Max);
}

void FVulkanCommandContext::SetScissor(IVector2 Offset, UVector2 Size)
{
    PendingState->SetScissor(Offset, Size);
}

void FVulkanCommandContext::Draw(uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances)
{
    FVulkanCmdBuffer* CmdBuffer = CommandManager->GetActiveCmdBuffer();
    PendingState->PrepareForDraw(CmdBuffer);
    VulkanAPI::vkCmdDraw(CmdBuffer->GetHandle(), NumPrimitives * 3, NumInstances, BaseVertexIndex, 0);
}

void FVulkanCommandContext::DrawIndexed(Ref<RRHIBuffer> InIndexBuffer, int32 BaseVertexIndex, uint32 FirstInstance,
                                        uint32 NumVertices, uint32 StartIndex, uint32 NumPrimitives,
                                        uint32 NumInstances)
{
    (void)NumVertices;
    FVulkanCmdBuffer* CmdBuffer = CommandManager->GetActiveCmdBuffer();
    PendingState->PrepareForDraw(CmdBuffer);

    RVulkanBuffer* const IndexBuffer = InIndexBuffer.AsRaw<RVulkanBuffer>();
    VulkanAPI::vkCmdBindIndexBuffer(CmdBuffer->GetHandle(), IndexBuffer->GetHandle(), 0, IndexBuffer->GetIndexType());
    VulkanAPI::vkCmdDrawIndexed(CmdBuffer->GetHandle(), NumPrimitives, NumInstances, StartIndex, BaseVertexIndex,
                                FirstInstance);
}

void FVulkanCommandContext::CopyRessourceArrayToBuffer(const IResourceArrayInterface* Source,
                                                       Ref<RRHIBuffer>& Destination, uint64 SourceOffset,
                                                       uint64 DestinationOffset, uint64 Size)
{
    RVulkanBuffer* const DstBuffer = Destination.AsRaw<RVulkanBuffer>();
    RVulkanMemoryAllocation* const Memory = DstBuffer->GetMemory();

    void* const MappedPtr = Memory->Map(Size, DestinationOffset);
    uint8 const* const SourceData = reinterpret_cast<uint8 const*>(Source->GetData());
    std::memcpy(MappedPtr, SourceData + SourceOffset, Size);
    Memory->Unmap();
    Memory->FlushMappedMemory(0, Size);
}

void FVulkanCommandContext::CopyBufferToBuffer(const Ref<RRHIBuffer>& Source, Ref<RRHIBuffer>& Destination,
                                               uint64 SourceOffset, uint64 DestinationOffset, uint64 Size)
{
    const RVulkanBuffer* const SrcBuffer = Source.AsRaw<RVulkanBuffer>();
    RVulkanBuffer* const DstBuffer = Destination.AsRaw<RVulkanBuffer>();
    const VkBufferCopy copyRegion{
        .srcOffset = SourceOffset,
        .dstOffset = DestinationOffset,
        .size = Size,
    };
    FVulkanCmdBuffer* CmdBuffer = CommandManager->GetUploadCmdBuffer();
    VulkanAPI::vkCmdCopyBuffer(CmdBuffer->GetHandle(), SrcBuffer->GetHandle(), DstBuffer->GetHandle(), 1, &copyRegion);
    CommandManager->SubmitUploadCmdBuffer();
}

void FVulkanCommandContext::SetLayout(RVulkanTexture* const Texture, VkImageLayout Layout)
{
    Texture->SetLayout(CommandManager->GetActiveCmdBuffer(), Layout);
}

}    // namespace VulkanRHI

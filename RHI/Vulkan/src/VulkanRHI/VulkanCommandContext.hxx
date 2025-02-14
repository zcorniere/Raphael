#pragma once

#include "Engine/Core/RHI/RHIContext.hxx"

namespace VulkanRHI
{

class FVulkanDevice;
class FVulkanQueue;
class FVulkanPendingState;
class VulkanCommandBufferManager;

class RVulkanTexture;

class FVulkanCommandContext : public FRHIContext
{
    RTTI_DECLARE_TYPEINFO(FVulkanCommandContext, FRHIContext);

public:
    FVulkanCommandContext(FVulkanDevice* InDevice, FVulkanQueue* InGraphicsQueue, FVulkanQueue* InPresentQueue);
    virtual ~FVulkanCommandContext();

    void Reset() override;

public:
    /// @brief Mark the beginning of a new frame
    virtual void BeginFrame() override;
    /// @brief Mark the end of the current frame
    virtual void EndFrame() override;

    /// @brief Indicate the RHI that we are starting drawing in the given viewport
    virtual void RHIBeginDrawingViewport(RRHIViewport* const Viewport) override;
    /// @brief Indicate the RHI that we are done drawing in the given viewport
    virtual void RHIEndDrawningViewport(RRHIViewport* const Viewport) override;

    virtual void RHIBeginRendering(const RHIRenderPassDescription& Description) override;
    virtual void RHIEndRendering() override;

    virtual void SetPipeline(Ref<RRHIGraphicsPipeline>& Pipeline) override;
    virtual void SetMaterial(Ref<RRHIMaterial>& Material) override;

    virtual void SetVertexBuffer(Ref<RRHIBuffer>& VertexBuffer, uint32 BufferIndex, uint32 Offset) override;

    virtual void SetViewport(FVector3 Min, FVector3 Max) override;
    virtual void SetScissor(IVector2 Offset, UVector2 Size) override;

    virtual void Draw(uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances) override;
    virtual void DrawIndexed(Ref<RRHIBuffer> InIndexBuffer, int32 BaseVertexIndex, uint32 FirstInstance,
                             uint32 NumVertices, uint32 StartIndex, uint32 NumPrimitives, uint32 NumInstances) override;

    virtual void CopyRessourceArrayToBuffer(const IResourceArrayInterface* Source, Ref<RRHIBuffer>& Destination,
                                            uint64 SourceOffset, uint64 DestinationOffset, uint64 Size) override;
    virtual void CopyBufferToBuffer(const Ref<RRHIBuffer>& Source, Ref<RRHIBuffer>& Destination, uint64 SourceOffset,
                                    uint64 DestinationOffset, uint64 Size) override;

    /// @brief VulkanRHI only, set the layout of the given texture
    void SetLayout(RVulkanTexture* const Texture, VkImageLayout Layout);

    VulkanCommandBufferManager* GetCommandManager() const
    {
        return CommandManager.get();
    }

    FVulkanPendingState* GetPendingState() const
    {
        return PendingState.get();
    }

private:
    std::unique_ptr<FVulkanPendingState> PendingState;
    std::unique_ptr<VulkanCommandBufferManager> CommandManager;

    FVulkanDevice* const Device = nullptr;
    FVulkanQueue* const GfxQueue = nullptr;
    FVulkanQueue* const PresentQueue = nullptr;
};

}    // namespace VulkanRHI

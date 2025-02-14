#pragma once

#include "Engine/Containers/ResourceArray.hxx"

class RRHIGraphicsPipeline;
class RRHIMaterial;
class RRHIBuffer;
class RRHIViewport;
struct RHIRenderPassDescription;

class FRHIContext : public RTTI::FEnable
{
    RTTI_DECLARE_TYPEINFO(FRHIContext);

public:
    virtual ~FRHIContext() = default;

    virtual void Reset() = 0;

    /// @brief Mark the beginning of a new frame
    virtual void BeginFrame() = 0;
    /// @brief Mark the end of the current frame
    virtual void EndFrame() = 0;

    /// @brief Indicate the RHI that we are starting drawing in the given viewport
    virtual void RHIBeginDrawingViewport(RRHIViewport* const Viewport) = 0;
    /// @brief Indicate the RHI that we are done drawing in the given viewport
    virtual void RHIEndDrawningViewport(RRHIViewport* const Viewport) = 0;

    /// @brief Begin rendering a new render pass
    virtual void RHIBeginRendering(const RHIRenderPassDescription& Description) = 0;
    /// @brief End rendering the current render pass
    virtual void RHIEndRendering() = 0;

    /// @brief Set the pipeline to use for the next draw calls
    virtual void SetPipeline(Ref<RRHIGraphicsPipeline>& Pipeline) = 0;
    virtual void SetMaterial(Ref<RRHIMaterial>& Material) = 0;

    virtual void SetVertexBuffer(Ref<RRHIBuffer>& VertexBuffer, uint32 BufferIndex, uint32 Offset) = 0;

    virtual void SetViewport(FVector3 Min, FVector3 Max) = 0;
    virtual void SetScissor(IVector2 Offset, UVector2 Size) = 0;

    /// @brief Send a draw call to the RHI
    virtual void Draw(uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances) = 0;
    virtual void DrawIndexed(Ref<RRHIBuffer> InIndexBuffer, int32 BaseVertexIndex, uint32 FirstInstance,
                             uint32 NumVertices, uint32 StartIndex, uint32 NumPrimitives, uint32 NumInstances) = 0;

    /// @brief Copy the content of a resource array to a buffer
    virtual void CopyRessourceArrayToBuffer(const IResourceArrayInterface* Source, Ref<RRHIBuffer>& Destination,
                                            uint64 SourceOffset, uint64 DestinationOffset, uint64 Size) = 0;
    /// @brief Copy the content of a buffer to another buffer
    virtual void CopyBufferToBuffer(const Ref<RRHIBuffer>& Source, Ref<RRHIBuffer>& Destination, uint64 SourceOffset,
                                    uint64 DestinationOffset, uint64 Size) = 0;
};

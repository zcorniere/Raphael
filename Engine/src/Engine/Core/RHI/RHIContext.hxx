#pragma once

#include "Engine/Core/RHI/Resources/RHIViewport.hxx"

class RHIContext : public RTTI::Enable
{
    RTTI_DECLARE_TYPEINFO(RHIContext);

public:
    virtual ~RHIContext() = default;

    virtual void Reset() = 0;

    /// @brief Mark the beginning of a new frame
    virtual void BeginFrame() = 0;
    /// @brief Mark the end of the current frame
    virtual void EndFrame() = 0;

    /// @brief Indicate the RHI that we are starting drawing in the given viewport
    virtual void RHIBeginDrawingViewport(RHIViewport* const Viewport) = 0;
    /// @brief Indicate the RHI that we are done drawing in the given viewport
    virtual void RHIEndDrawningViewport(RHIViewport* const Viewport) = 0;

    /// @brief Begin rendering a new render pass
    virtual void RHIBeginRendering(const RHIRenderPassDescription& Description) = 0;
    /// @brief End rendering the current render pass
    virtual void RHIEndRendering() = 0;

    /// @brief Set the pipeline to use for the next draw calls
    virtual void SetPipeline(Ref<RHIGraphicsPipeline>& Pipeline) = 0;

    virtual void SetVertexBuffer(Ref<RHIBuffer>& VertexBuffer, uint32 BufferIndex, uint32 Offset) = 0;

    virtual void SetViewport(glm::vec3 Min, glm::vec3 Max) = 0;
    virtual void SetScissor(glm::ivec2 Offset, glm::uvec2 Size) = 0;

    /// @brief Send a draw call to the RHI
    virtual void Draw(uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances) = 0;

    /// @brief Copy the content of a buffer to another buffer
    virtual void CopyBufferToBuffer(const Ref<RHIBuffer>& Source, Ref<RHIBuffer>& Destination, uint64 SourceOffset,
                                    uint64 DestinationOffset, uint64 Size) = 0;
};

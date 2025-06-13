#include "Engine/Core/RHI/RHICommand.hxx"

void FRHIBeginFrame::Execute(FFRHICommandList& CommandList)
{
    CommandList.GetContext()->BeginFrame();
}

void FRHIEndFrame::Execute(FFRHICommandList& CommandList)
{
    CommandList.GetContext()->EndFrame();
}

FRHIBeginDrawingViewport::FRHIBeginDrawingViewport(Ref<RRHIViewport> InViewport): Viewport(std::move(InViewport))
{
}
void FRHIBeginDrawingViewport::Execute(FFRHICommandList& CommandList)
{
    CommandList.GetContext()->RHIBeginDrawingViewport(Viewport.Raw());
}

FRHIEndDrawningViewport::FRHIEndDrawningViewport(Ref<RRHIViewport> InViewport): Viewport(std::move(InViewport))
{
}
void FRHIEndDrawningViewport::Execute(FFRHICommandList& CommandList)
{
    CommandList.GetContext()->RHIEndDrawningViewport(Viewport.Raw());
}

FRHIBeginRendering::FRHIBeginRendering(const FRHIRenderPassDescription& InDescription): Description(InDescription)
{
}

void FRHIBeginRendering::Execute(FFRHICommandList& CommandList)
{
    CommandList.GetContext()->RHIBeginRendering(Description);
}

void FRHIEndRendering::Execute(FFRHICommandList& CommandList)
{
    CommandList.GetContext()->RHIEndRendering();
}

FRHISetMaterial::FRHISetMaterial(Ref<RRHIMaterial> InMaterial): Material(std::move(InMaterial))
{
}

void FRHISetMaterial::Execute(FFRHICommandList& CommandList)
{
    CommandList.GetContext()->SetMaterial(Material);
}

FRHISetGraphicsPipeline::FRHISetGraphicsPipeline(Ref<RRHIGraphicsPipeline> InPipeline): Pipeline(std::move(InPipeline))
{
}

void FRHISetGraphicsPipeline::Execute(FFRHICommandList& CommandList)
{
    CommandList.GetContext()->SetPipeline(Pipeline);
}

FRHISetVertexBuffer::FRHISetVertexBuffer(Ref<RRHIBuffer> InVertexBuffer, uint32 BufferIndex, uint32 Offset)
    : VertexBuffer(std::move(InVertexBuffer))
    , BufferIndex(BufferIndex)
    , Offset(Offset)
{
    check(EnumHasAnyFlags(EBufferUsageFlags::VertexBuffer, VertexBuffer->GetUsage()));
}

void FRHISetVertexBuffer::Execute(FFRHICommandList& CommandList)
{
    CommandList.GetContext()->SetVertexBuffer(VertexBuffer, BufferIndex, Offset);
}

FRHISetViewport::FRHISetViewport(FVector3 Min, FVector3 Max): Min(Min), Max(Max)
{
}

void FRHISetViewport::Execute(FFRHICommandList& CommandList)
{
    CommandList.GetContext()->SetViewport(Min, Max);
}

FRHISetScissor::FRHISetScissor(IVector2 Offset, UVector2 Size): Offset(Offset), Size(Size)
{
}

void FRHISetScissor::Execute(FFRHICommandList& CommandList)
{
    CommandList.GetContext()->SetScissor(Offset, Size);
}

FRHIDraw::FRHIDraw(uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances)
    : BaseVertexIndex(BaseVertexIndex)
    , NumPrimitives(NumPrimitives)
    , NumInstances(NumInstances)
{
}

void FRHIDraw::Execute(FFRHICommandList& CommandList)
{
    CommandList.GetContext()->Draw(BaseVertexIndex, NumPrimitives, NumInstances);
}

RHIDrawIndexed::RHIDrawIndexed(Ref<RRHIBuffer> InIndexBuffer, int32 InBaseVertexIndex, uint32 InFirstInstance,
                               uint32 InNumVertices, uint32 InStartIndex, uint32 InNumPrimitives, uint32 InNumInstances)
    : IndexBuffer(std::move(InIndexBuffer))
    , BaseVertexIndex(InBaseVertexIndex)
    , FirstInstance(InFirstInstance)
    , NumVertices(InNumVertices)
    , StartIndex(InStartIndex)
    , NumPrimitives(InNumPrimitives)
    , NumInstances(InNumInstances)
{
    check(EnumHasAnyFlags(EBufferUsageFlags::IndexBuffer, IndexBuffer->GetUsage()));
}

void RHIDrawIndexed::Execute(FFRHICommandList& CommandList)
{
    CommandList.GetContext()->DrawIndexed(IndexBuffer, BaseVertexIndex, FirstInstance, NumVertices, StartIndex,
                                          NumPrimitives, NumInstances);
}

RHICopyResourceArrayToBuffer::RHICopyResourceArrayToBuffer(IResourceArrayInterface* const InSourceArray,
                                                           Ref<RRHIBuffer> InDestinationBuffer, uint64 InSourceOffset,
                                                           uint64 InDestinationOffset, uint64 InSize)
    : SourceArray(InSourceArray)
    , DestinationBuffer(std::move(InDestinationBuffer))
    , Size(InSize)
    , SourceOffset(InSourceOffset)
    , DestinationOffset(InDestinationOffset)
{
    check(EnumHasAnyFlags(EBufferUsageFlags::KeepCPUAccessible, DestinationBuffer->GetUsage()));
}

void RHICopyResourceArrayToBuffer::Execute(FFRHICommandList& CommandList)
{
    CommandList.GetContext()->CopyResourceArrayToBuffer(SourceArray, DestinationBuffer, SourceOffset, DestinationOffset,
                                                        Size);
}

RHICopyBufferToBuffer::RHICopyBufferToBuffer(const Ref<RRHIBuffer> InSourceBuffer, Ref<RRHIBuffer> InDestinationBuffer,
                                             uint64 InSourceOffset, uint64 InDestinationOffset, uint64 InSize)
    : SourceBuffer(std::move(InSourceBuffer))
    , DestinationBuffer(std::move(InDestinationBuffer))
    , Size(InSize)
    , SourceOffset(InSourceOffset)
    , DestinationOffset(InDestinationOffset)
{
    check(EnumHasAnyFlags(EBufferUsageFlags::SourceCopy, SourceBuffer->GetUsage()));
    check(EnumHasAnyFlags(EBufferUsageFlags::DestinationCopy, DestinationBuffer->GetUsage()));
}

void RHICopyBufferToBuffer::Execute(FFRHICommandList& CommandList)
{
    CommandList.GetContext()->CopyBufferToBuffer(SourceBuffer, DestinationBuffer, SourceOffset, DestinationOffset,
                                                 Size);
}

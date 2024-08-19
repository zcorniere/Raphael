#include "Engine/Core/RHI/RHICommand.hxx"

void RHIBeginFrame::Execute(RHICommandList& CommandList)
{
    CommandList.GetContext()->BeginFrame();
}

void RHIEndFrame::Execute(RHICommandList& CommandList)
{
    CommandList.GetContext()->EndFrame();
}

RHIBeginDrawingViewport::RHIBeginDrawingViewport(Ref<RHIViewport> InViewport): Viewport(std::move(InViewport))
{
}
void RHIBeginDrawingViewport::Execute(RHICommandList& CommandList)
{
    CommandList.GetContext()->RHIBeginDrawingViewport(Viewport.Raw());
}

RHIEndDrawningViewport::RHIEndDrawningViewport(Ref<RHIViewport> InViewport): Viewport(std::move(InViewport))
{
}
void RHIEndDrawningViewport::Execute(RHICommandList& CommandList)
{
    CommandList.GetContext()->RHIEndDrawningViewport(Viewport.Raw());
}

RHIBeginRendering::RHIBeginRendering(const RHIRenderPassDescription& InDescription): Description(InDescription)
{
}

void RHIBeginRendering::Execute(RHICommandList& CommandList)
{
    CommandList.GetContext()->RHIBeginRendering(Description);
}

void RHIEndRendering::Execute(RHICommandList& CommandList)
{
    CommandList.GetContext()->RHIEndRendering();
}

RHISetPipeline::RHISetPipeline(Ref<RHIGraphicsPipeline> InPipeline): Pipeline(std::move(InPipeline))
{
}

void RHISetPipeline::Execute(RHICommandList& CommandList)
{
    CommandList.GetContext()->SetPipeline(Pipeline);
}

RHISetVertexBuffer::RHISetVertexBuffer(Ref<RHIBuffer> InVertexBuffer, uint32 BufferIndex, uint32 Offset)
    : VertexBuffer(std::move(InVertexBuffer)), BufferIndex(BufferIndex), Offset(Offset)
{
    check(EnumHasAnyFlags(EBufferUsageFlags::VertexBuffer, VertexBuffer->GetUsage()));
}

void RHISetVertexBuffer::Execute(RHICommandList& CommandList)
{
    CommandList.GetContext()->SetVertexBuffer(VertexBuffer, BufferIndex, Offset);
}

RHISetViewport::RHISetViewport(FVector3 Min, FVector3 Max): Min(Min), Max(Max)
{
}

void RHISetViewport::Execute(RHICommandList& CommandList)
{
    CommandList.GetContext()->SetViewport(Min, Max);
}

RHISetScissor::RHISetScissor(IVector2 Offset, UVector2 Size): Offset(Offset), Size(Size)
{
}

void RHISetScissor::Execute(RHICommandList& CommandList)
{
    CommandList.GetContext()->SetScissor(Offset, Size);
}

RHIDraw::RHIDraw(uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances)
    : BaseVertexIndex(BaseVertexIndex), NumPrimitives(NumPrimitives), NumInstances(NumInstances)
{
}

void RHIDraw::Execute(RHICommandList& CommandList)
{
    CommandList.GetContext()->Draw(BaseVertexIndex, NumPrimitives, NumInstances);
}

RHIDrawIndexed::RHIDrawIndexed(Ref<RHIBuffer> InIndexBuffer, int32 InBaseVertexIndex, uint32 InFirstInstance,
                               uint32 InNumVertices, uint32 InStartIndex, uint32 InNumPrimitives, uint32 InNumInstances)
    : IndexBuffer(std::move(InIndexBuffer)),
      BaseVertexIndex(InBaseVertexIndex),
      FirstInstance(InFirstInstance),
      NumVertices(InNumVertices),
      StartIndex(InStartIndex),
      NumPrimitives(InNumPrimitives),
      NumInstances(InNumInstances)
{
    check(EnumHasAnyFlags(EBufferUsageFlags::IndexBuffer, IndexBuffer->GetUsage()));
}

void RHIDrawIndexed::Execute(RHICommandList& CommandList)
{
    CommandList.GetContext()->DrawIndexed(IndexBuffer, BaseVertexIndex, FirstInstance, NumVertices, StartIndex,
                                          NumPrimitives, NumInstances);
}

RHICopyBufferToBuffer::RHICopyBufferToBuffer(const Ref<RHIBuffer> InSourceBuffer, Ref<RHIBuffer> InDestinationBuffer,
                                             uint64 InSourceOffset, uint64 InDestinationOffset, uint64 InSize)
    : SourceBuffer(std::move(InSourceBuffer)),
      DestinationBuffer(std::move(InDestinationBuffer)),
      Size(InSize),
      SourceOffset(InSourceOffset),
      DestinationOffset(InDestinationOffset)
{
    check(EnumHasAnyFlags(EBufferUsageFlags::SourceCopy, SourceBuffer->GetUsage()));
    check(EnumHasAnyFlags(EBufferUsageFlags::DestinationCopy, DestinationBuffer->GetUsage()));
}

void RHICopyBufferToBuffer::Execute(RHICommandList& CommandList)
{
    CommandList.GetContext()->CopyBufferToBuffer(SourceBuffer, DestinationBuffer, SourceOffset, DestinationOffset,
                                                 Size);
}

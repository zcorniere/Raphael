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

RHIEndDrawningViewport::RHIEndDrawningViewport(Ref<RHIViewport> InViewport, bool bInPresent)
    : Viewport(std::move(InViewport)), bPresent(bInPresent)
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

RHISetViewport::RHISetViewport(glm::vec3 Min, glm::vec3 Max): Min(Min), Max(Max)
{
}

void RHISetViewport::Execute(RHICommandList& CommandList)
{
    CommandList.GetContext()->SetViewport(Min, Max);
}

RHISetScissor::RHISetScissor(glm::ivec2 Offset, glm::uvec2 Size): Offset(Offset), Size(Size)
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

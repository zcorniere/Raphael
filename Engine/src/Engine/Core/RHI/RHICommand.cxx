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

#include "Engine/Core/RHI/RHICommand.hxx"

void RHIBeginFrame::Execute(RHICommandList& CommandList)
{
    CommandList.GetContext()->BeginFrame();
}

void RHIEndFrame::Execute(RHICommandList& CommandList)
{
    CommandList.GetContext()->EndFrame();
}

RHIBeginDrawingViewport::RHIBeginDrawingViewport(WeakRef<RHIViewport> InViewport): Viewport(InViewport)
{
}
void RHIBeginDrawingViewport::Execute(RHICommandList& CommandList)
{
    CommandList.GetContext()->RHIBeginDrawingViewport(Viewport);
}

RHIEndDrawningViewport::RHIEndDrawningViewport(WeakRef<RHIViewport> InViewport, bool bInPresent)
    : Viewport(InViewport), bPresent(bInPresent)
{
}
void RHIEndDrawningViewport::Execute(RHICommandList& CommandList)
{
    CommandList.GetContext()->RHIEndDrawningViewport(Viewport);
}

RHIResizeViewport::RHIResizeViewport(WeakRef<RHIViewport> InViewport, uint32_t InWidth, uint32_t InHeight)
    : Viewport(InViewport), Width(InWidth), Height(InHeight)
{
}
void RHIResizeViewport::Execute(RHICommandList& CommandList)
{
    CommandList.GetContext()->RHIResizeViewport(Viewport, Width, Height);
}

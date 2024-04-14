#include "Engine/Core/RHI/RHICommandList.hxx"
#include "Engine/Core/RHI/GenericRHI.hxx"
#include "Engine/Core/RHI/RHICommand.hxx"

RHICommandListExecutor GCommandListExecutor;

RHICommandList::RHICommandList()
{
}

RHICommandList::~RHICommandList()
{
    Reset();
}

void RHICommandList::BeginFrame()
{
    Enqueue(new RHIBeginFrame());
}
void RHICommandList::EndFrame()
{
    Enqueue(new RHIEndFrame());
}

void RHICommandList::ResizeViewport(RHIViewport* Viewport, uint32_t Width, uint32_t Height)
{
    Enqueue(new RHIResizeViewport(Viewport, Width, Height));
}

void RHICommandList::BeginRenderingViewport(RHIViewport* Viewport)
{
    Enqueue(new RHIBeginDrawingViewport(Viewport));
}
void RHICommandList::EndRenderingViewport(RHIViewport* Viewport, bool bPresent)
{
    Enqueue(new RHIEndDrawningViewport(Viewport, bPresent));
}

void RHICommandList::Enqueue(RHIRenderCommandBase* RenderCommand)
{
    // If we are executing the command list, we need to execute the command immediately
    if (bIsExecuting) {
        check(m_Context != nullptr);
        RenderCommand->DoTask(*this);
        return;
    }

    if (m_CommandList == nullptr) {
        m_CommandList.reset(RenderCommand);
        m_CommandListTail = m_CommandList.get();
    } else {
        m_CommandListTail->p_Next.reset(RenderCommand);
        m_CommandListTail = m_CommandListTail->p_Next.get();
    }
    check(m_CommandListTail != nullptr);
}

void RHICommandList::Execute(RHIContext* const InContext)
{
    m_Context = InContext;
    check(m_Context != nullptr);

    // Finish the command list with a submit command
    struct FinalizeAndSubmitCommandListString {
        static constexpr const char* Str()
        {
            return "FinalizeAndSubmitCommandList";
        }
    };
    this->EnqueueLambda<FinalizeAndSubmitCommandListString>(
        [](RHICommandList& CommandList) { RHI::Get()->RHISubmitCommandLists(&CommandList, 1); });

    // Execute all the commands
    bIsExecuting = true;
    RHIRenderCommandBase* Command = m_CommandList.get();
    while (Command != nullptr) {
        Command->DoTask(*this);
        Command = Command->p_Next.get();
    }
    bIsExecuting = false;

    // TODO: reset the list during iteration
    Reset();
    m_Context = nullptr;
}

void RHICommandList::Reset()
{
    // We need to be very careful with deleting the chained list
    // By doing it that way we are to destroy one node at a time
    while (m_CommandList != nullptr) {
        m_CommandList = std::move(m_CommandList->p_Next);
    }
    m_CommandListTail = nullptr;
}

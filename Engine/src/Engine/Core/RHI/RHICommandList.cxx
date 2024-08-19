#include "Engine/Core/RHI/RHICommandList.hxx"
#include "Engine/Core/RHI/GenericRHI.hxx"
#include "Engine/Core/RHI/RHICommand.hxx"

RHICommandList::RHICommandList()
{
}

RHICommandList::~RHICommandList()
{
    Reset();

    /// Don't forget to release the context
    if (m_Context != nullptr) {
        RHI::Get()->RHIReleaseCommandContext(m_Context);
    }
}

void RHICommandList::BeginFrame()
{
    Enqueue(new RHIBeginFrame());
}
void RHICommandList::EndFrame()
{
    Enqueue(new RHIEndFrame());
}

void RHICommandList::BeginRenderingViewport(RHIViewport* Viewport)
{
    Enqueue(new RHIBeginDrawingViewport(Viewport));
}
void RHICommandList::EndRenderingViewport(RHIViewport* Viewport)
{
    Enqueue(new RHIEndDrawningViewport(Viewport));
}

void RHICommandList::BeginRendering(const RHIRenderPassDescription& Description)
{
    Enqueue(new RHIBeginRendering(Description));
}
void RHICommandList::EndRendering()
{
    Enqueue(new RHIEndRendering());
}

void RHICommandList::SetPipeline(Ref<RHIGraphicsPipeline>& Pipeline)
{
    Enqueue(new RHISetPipeline(Pipeline));
}

void RHICommandList::SetVertexBuffer(Ref<RHIBuffer>& VertexBuffer, uint32 BufferIndex, uint32 Offset)
{
    Enqueue(new RHISetVertexBuffer(VertexBuffer, BufferIndex, Offset));
}

void RHICommandList::SetViewport(FVector3 Min, FVector3 Max)
{
    Enqueue(new RHISetViewport(Min, Max));
}

void RHICommandList::SetScissor(IVector2 Offset, UVector2 Size)
{
    Enqueue(new RHISetScissor(Offset, Size));
}

void RHICommandList::Draw(uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances)
{
    Enqueue(new RHIDraw(BaseVertexIndex, NumPrimitives, NumInstances));
}

void RHICommandList::DrawIndexed(Ref<RHIBuffer>& IndexBuffer, int32 BaseVertexIndex, uint32 FirstInstance,
                                 uint32 NumVertices, uint32 StartIndex, uint32 NumPrimitives, uint32 NumInstances)
{
    Enqueue(new RHIDrawIndexed(IndexBuffer, BaseVertexIndex, FirstInstance, NumVertices, StartIndex, NumPrimitives,
                               NumInstances));
}

void RHICommandList::CopyBufferToBuffer(const Ref<RHIBuffer>& Source, Ref<RHIBuffer>& Destination, uint64 SourceOffset,
                                        uint64 DestinationOffset, uint64 Size)
{
    Enqueue(new RHICopyBufferToBuffer(Source, Destination, SourceOffset, DestinationOffset, Size));
}

void RHICommandList::Enqueue(RHIRenderCommandBase* RenderCommand)
{
    // If we are executing the command list, we need to execute the command immediately
    if (bIsExecuting) {
        check(m_Context != nullptr);
        RenderCommand->DoTask(*this);
        delete RenderCommand;
        return;
    }

    if (m_CommandList == nullptr) {
        m_CommandList = RenderCommand;
        m_CommandListTail = m_CommandList;
    } else {
        m_CommandListTail->p_Next = RenderCommand;
        m_CommandListTail = m_CommandListTail->p_Next;
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
    m_CommandListTail = nullptr;
    while (m_CommandList != nullptr) {
        m_CommandList->DoTask(*this);

        // Delete the object after grabbing a ref to the next one
        RHIRenderCommandBase* const Next = m_CommandList->p_Next;
        delete m_CommandList;
        m_CommandList = Next;
    }
    bIsExecuting = false;

    // Should not do anything, but just in case
    Reset();
    m_Context = nullptr;
}

void RHICommandList::Reset()
{
    // We need to be very careful with deleting the chained list
    // By doing it that way we are to destroy one node at a time

    while (m_CommandList != nullptr) {
        RHIRenderCommandBase* const Next = m_CommandList->p_Next;

        delete m_CommandList;
        m_CommandList = Next;
    }
    m_CommandList = nullptr;
    m_CommandListTail = nullptr;
}

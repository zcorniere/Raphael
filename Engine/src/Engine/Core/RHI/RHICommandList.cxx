#include "Engine/Core/RHI/RHICommandList.hxx"
#include "Engine/Core/RHI/GenericRHI.hxx"
#include "Engine/Core/RHI/RHICommand.hxx"

FFRHICommandList::FFRHICommandList()
{
}

FFRHICommandList::~FFRHICommandList()
{
    Reset();

    /// Don't forget to release the context
    if (m_Context != nullptr)
    {
        RHI::Get()->RHIReleaseCommandContext(m_Context);
    }
}

void FFRHICommandList::BeginFrame()
{
    Enqueue(new FRHIBeginFrame());
}
void FFRHICommandList::EndFrame()
{
    Enqueue(new FRHIEndFrame());
}

void FFRHICommandList::BeginRenderingViewport(RRHIViewport* Viewport)
{
    Enqueue(new FRHIBeginDrawingViewport(Viewport));
}
void FFRHICommandList::EndRenderingViewport(RRHIViewport* Viewport)
{
    Enqueue(new FRHIEndDrawningViewport(Viewport));
}

void FFRHICommandList::BeginRendering(const FRHIRenderPassDescription& Description)
{
    Enqueue(new FRHIBeginRendering(Description));
}
void FFRHICommandList::EndRendering()
{
    Enqueue(new FRHIEndRendering());
}

void FFRHICommandList::SetMaterial(const Ref<RRHIMaterial>& Material)
{
    Enqueue(new FRHISetMaterial(Material));
}

void FFRHICommandList::SetVertexBuffer(const Ref<RRHIBuffer>& VertexBuffer, uint32 BufferIndex, uint32 Offset)
{
    Enqueue(new FRHISetVertexBuffer(VertexBuffer, BufferIndex, Offset));
}

void FFRHICommandList::SetViewport(FVector3 Min, FVector3 Max)
{
    Enqueue(new FRHISetViewport(Min, Max));
}

void FFRHICommandList::SetScissor(IVector2 Offset, UVector2 Size)
{
    Enqueue(new FRHISetScissor(Offset, Size));
}

void FFRHICommandList::Draw(uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances)
{
    Enqueue(new FRHIDraw(BaseVertexIndex, NumPrimitives, NumInstances));
}

void FFRHICommandList::DrawIndexed(const Ref<RRHIBuffer>& IndexBuffer, int32 BaseVertexIndex, uint32 FirstInstance,
                                   uint32 NumVertices, uint32 StartIndex, uint32 NumPrimitives, uint32 NumInstances)
{
    Enqueue(new RHIDrawIndexed(IndexBuffer, BaseVertexIndex, FirstInstance, NumVertices, StartIndex, NumPrimitives,
                               NumInstances));
}

void FFRHICommandList::CopyBufferToBuffer(const Ref<RRHIBuffer>& Source, Ref<RRHIBuffer>& Destination,
                                          uint64 SourceOffset, uint64 DestinationOffset, uint64 Size)
{
    Enqueue(new RHICopyBufferToBuffer(Source, Destination, SourceOffset, DestinationOffset, Size));
}

void FFRHICommandList::CopyResourceArrayToBuffer(IResourceArrayInterface* Source, Ref<RRHIBuffer>& Destination,
                                                 uint64 SourceOffset, uint64 DestinationOffset, uint64 Size)
{
    Enqueue(new RHICopyResourceArrayToBuffer(Source, Destination, SourceOffset, DestinationOffset, Size));
}

void FFRHICommandList::Enqueue(FRHIRenderCommandBase* RenderCommand)
{
    // If we are executing the command list, we need to execute the command immediately
    if (bIsExecuting)
    {
        check(m_Context != nullptr);
        RenderCommand->DoTask(*this);
        delete RenderCommand;
        return;
    }

    if (m_CommandList == nullptr)
    {
        m_CommandList = RenderCommand;
        m_CommandListTail = m_CommandList;
    }
    else
    {
        m_CommandListTail->p_Next = RenderCommand;
        m_CommandListTail = m_CommandListTail->p_Next;
    }
    check(m_CommandListTail != nullptr);
}

void FFRHICommandList::Execute(FRHIContext* const InContext)
{
    m_Context = InContext;
    check(m_Context != nullptr);

    // Finish the command list with a submit command
    struct FinalizeAndSubmitCommandListString
    {
        static constexpr const char* Str()
        {
            return "FinalizeAndSubmitCommandList";
        }
    };
    this->EnqueueLambda<FinalizeAndSubmitCommandListString>([](FFRHICommandList& CommandList)
                                                            { RHI::Get()->RHISubmitCommandLists(&CommandList, 1); });

    // Execute all the commands
    bIsExecuting = true;
    m_CommandListTail = nullptr;
    while (m_CommandList != nullptr)
    {
        m_CommandList->DoTask(*this);

        // Delete the object after grabbing a ref to the next one
        FRHIRenderCommandBase* const Next = m_CommandList->p_Next;
        delete m_CommandList;
        m_CommandList = Next;
    }
    bIsExecuting = false;

    // Should not do anything, but just in case
    Reset();
    m_Context = nullptr;
}

void FFRHICommandList::Reset()
{
    // We need to be very careful with deleting the chained list
    // By doing it that way we are to destroy one node at a time

    while (m_CommandList != nullptr)
    {
        FRHIRenderCommandBase* const Next = m_CommandList->p_Next;

        delete m_CommandList;
        m_CommandList = Next;
    }
    m_CommandList = nullptr;
    m_CommandListTail = nullptr;
}

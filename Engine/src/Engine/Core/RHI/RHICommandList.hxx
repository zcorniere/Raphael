#pragma once

#include "Engine/Core/RHI/RHI.hxx"
#include "Engine/Core/RHI/RHIContext.hxx"

#define ENQUEUE_RENDER_COMMAND(Type)                \
    struct MACRO_EXPENDER(Type##String, __LINE__) { \
        static constexpr const char* Str()          \
        {                                           \
            return #Type;                           \
        }                                           \
    };                                              \
    RHICommandListExecutor::GetCommandList().EnqueueLambda<MACRO_EXPENDER(Type##String, __LINE__)>

DECLARE_LOGGER_CATEGORY(Core, LogRenderCommand, Warning)

class RHICommandList;

/// @brief Base class for all render commands
class RHIRenderCommandBase
{
public:
    virtual ~RHIRenderCommandBase() = default;

    virtual void DoTask(RHICommandList&) = 0;

public:
    /// @brief Pointer to the next command
    /// @note The current object own the next object
    RHIRenderCommandBase* p_Next = nullptr;
};

struct MissingNameCommand {
    static const char* Str()
    {
        return "MissingNameCommand";
    }
};

template <typename TNameType = MissingNameCommand>
class TRHIRenderCommand : public RHIRenderCommandBase
{
public:
    TRHIRenderCommand() = default;
    virtual ~TRHIRenderCommand() = default;

    virtual void DoTask(RHICommandList& CommandList) override final
    {
        RPH_PROFILE_SCOPE_DYNAMIC(TNameType::Str());
        LOG(LogRenderCommand, Trace, "Running task: {:s}", TNameType::Str());

        Execute(CommandList);
    }

    virtual void Execute(RHICommandList&) = 0;
};

template <typename TTypeString, typename TLambda>
class TLambdaRenderCommandType : public RHIRenderCommandBase
{
public:
    TLambdaRenderCommandType(TLambda&& InLambda): Lambda(std::forward<TLambda>(InLambda))
    {
    }
    virtual ~TLambdaRenderCommandType() = default;

    virtual void DoTask(RHICommandList& CommandList) override final
    {
        RPH_PROFILE_SCOPE_DYNAMIC(TTypeString::Str());
        LOG(LogRenderCommand, Trace, "Running task: {:s}", TTypeString::Str());
        Lambda(CommandList);
    }

private:
    TLambda Lambda;
};

class RHICommandList : public NamedClass
{

public:
    RHICommandList();
    ~RHICommandList();

public:
    /// @brief Begin a new frame
    void BeginFrame();
    /// @brief End the current frame
    void EndFrame();

    /// @brief Mark the given viewport as the current drawing target
    void BeginRenderingViewport(RHIViewport* Viewport);
    /// @brief Stop rendering to the given viewport and present it
    void EndRenderingViewport(RHIViewport* Viewport);

    /// @brief Begin a new rendering pass
    void BeginRendering(const RHIRenderPassDescription& Description);
    /// @brief End the current rendering pass
    void EndRendering();

    /// @brief Set the current pipeline
    void SetPipeline(Ref<RHIGraphicsPipeline>& Pipeline);
    /// @brief Set the vertex buffer
    void SetVertexBuffer(Ref<RHIBuffer>& VertexBuffer, uint32 BufferIndex = 0, uint32 Offset = 0);

    /// @brief Set the viewport used to render
    void SetViewport(FVector3 Min, FVector3 Max);
    /// @brief Set the scissor rectangle
    void SetScissor(IVector2 Offset, UVector2 Size);

    /// @brief Draw to the current render target
    ///
    /// @param BaseVertexIndex The index of the first vertex to draw
    /// @param NumPrimitives The number of primitives to draw
    /// @param NumInstances The number of instances to draw
    void Draw(uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances);
    /// @brief Draw to the current render target using an index buffer
    ///
    /// @param IndexBuffer The buffer containing the indices
    /// @param BaseVertexIndex The index of the first vertex to draw
    /// @param FirstInstance The index of the first instance to draw
    /// @param NumVertices The number of vertices to draw
    /// @param StartIndex The index of the first index to draw
    /// @param NumPrimitives The number of primitives to draw (triangles = 3 vertices = 1 primitive)
    /// @param NumInstances The number of instances to draw
    ///
    /// @note We only support Triangle primitive type, so the num of primitive is the number of triangles to draw
    void DrawIndexed(Ref<RHIBuffer>& IndexBuffer, int32 BaseVertexIndex, uint32 FirstInstance, uint32 NumVertices,
                     uint32 StartIndex, uint32 NumPrimitives, uint32 NumInstances);

    /// @brief Copy the content of a buffer to another buffer
    ///
    /// @param Source The buffer to copy from
    /// @param Destination The buffer to copy to
    /// @param SourceOffset The offset in the source buffer
    /// @param DestinationOffset The offset in the destination buffer
    /// @param Size The number of bytes to copy
    void CopyBufferToBuffer(const Ref<RHIBuffer>& Source, Ref<RHIBuffer>& Destination, uint64 SourceOffset,
                            uint64 DestinationOffset, uint64 Size);

    /// @brief Add a command to the back of the queue
    template <typename TSTR, typename TFunction>
    requires std::is_invocable_v<TFunction, RHICommandList&>
    void EnqueueLambda(TFunction&& Function)
    {
        return Enqueue(new TLambdaRenderCommandType<TSTR, TFunction>(std::forward<TFunction>(Function)));
    }

    /// @brief Execute the command in the given context
    void Execute(RHIContext* const Context);

    RHIContext* GetContext() const
    {
        return m_Context;
    }

private:
    /// @brief Add a command to the back of the queue
    /// @note Take ownership of the command
    void Enqueue(RHIRenderCommandBase* RenderCommand);

    /// @brief Reset the command list, deleting all the commands
    void Reset();

private:
    RHIContext* m_Context = nullptr;

    bool bIsExecuting = false;
    /// @brief Head of the command queue
    /// @note "this" has ownership of the object. Can't use unique_ptr because of some ""issues"" with destroying the
    /// list without stack overflow
    RHIRenderCommandBase* m_CommandList = nullptr;

    /// @brief Tail of the command queue
    RHIRenderCommandBase* m_CommandListTail = nullptr;
};

class RHICommandListExecutor
{
public:
    static RHICommandListExecutor& Get()
    {
        static RHICommandListExecutor Instance;
        return Instance;
    }

    static RHICommandList& GetCommandList()
    {
        return Get().m_CommandList;
    }

private:
    RHICommandList m_CommandList;
};

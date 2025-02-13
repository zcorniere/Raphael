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
    FRHICommandListExecutor::GetCommandList().EnqueueLambda<MACRO_EXPENDER(Type##String, __LINE__)>

DECLARE_LOGGER_CATEGORY(Core, LogRenderCommand, Warning)

class FFRHICommandList;

/// @brief Base class for all render commands
class FRHIRenderCommandBase
{
public:
    virtual ~FRHIRenderCommandBase() = default;

    virtual void DoTask(FFRHICommandList&) = 0;

public:
    /// @brief Pointer to the next command
    /// @note The current object own the next object
    FRHIRenderCommandBase* p_Next = nullptr;
};

struct FMissingNameCommand {
    static const char* Str()
    {
        return "MissingNameCommand";
    }
};

template <typename TNameType = FMissingNameCommand>
class TRHIRenderCommand : public FRHIRenderCommandBase
{
public:
    TRHIRenderCommand() = default;
    virtual ~TRHIRenderCommand() = default;

    virtual void DoTask(FFRHICommandList& CommandList) override final
    {
        RPH_PROFILE_SCOPE_DYNAMIC(TNameType::Str());
        LOG(LogRenderCommand, Trace, "Running task: {:s}", TNameType::Str());

        Execute(CommandList);
    }

    virtual void Execute(FFRHICommandList&) = 0;
};

template <typename TTypeString, typename TLambda, typename... ArgsType>
class TLambdaRenderCommandType : public FRHIRenderCommandBase
{
public:
    TLambdaRenderCommandType(TLambda&& InLambda, ArgsType&&... Args)
        : Lambda(std::forward<TLambda>(InLambda)), Args(std::make_tuple(std::forward<ArgsType>(Args)...))
    {
    }
    TLambdaRenderCommandType(const TLambdaRenderCommandType&) = delete;

    virtual ~TLambdaRenderCommandType() = default;

    virtual void DoTask(FFRHICommandList& CommandList) override final
    {
        RPH_PROFILE_SCOPE_DYNAMIC(TTypeString::Str());
        LOG(LogRenderCommand, Trace, "Running task: {:s}", TTypeString::Str());
        DoTaskImpl(CommandList, std::index_sequence_for<ArgsType...>{});
    }

private:
    template <std::size_t... Is>
    void DoTaskImpl(FFRHICommandList& CommandList, std::index_sequence<Is...>)
    {
        Lambda(CommandList, std::get<Is>(Args)...);
    }

    TLambda Lambda;
    std::tuple<std::decay_t<ArgsType>...> Args;
};

class FFRHICommandList : public FNamedClass
{

public:
    FFRHICommandList();
    ~FFRHICommandList();

public:
    /// @brief Begin a new frame
    void BeginFrame();
    /// @brief End the current frame
    void EndFrame();

    /// @brief Mark the given viewport as the current drawing target
    void BeginRenderingViewport(RRHIViewport* Viewport);
    /// @brief Stop rendering to the given viewport and present it
    void EndRenderingViewport(RRHIViewport* Viewport);

    /// @brief Begin a new rendering pass
    void BeginRendering(const RHIRenderPassDescription& Description);
    /// @brief End the current rendering pass
    void EndRendering();

    /// @brief Set the current pipeline
    void SetPipeline(const Ref<RRHIGraphicsPipeline>& Pipeline);
    /// @brief Set the vertex buffer
    void SetVertexBuffer(const Ref<RRHIBuffer>& VertexBuffer, uint32 BufferIndex = 0, uint32 Offset = 0);

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
    void DrawIndexed(const Ref<RRHIBuffer>& IndexBuffer, int32 BaseVertexIndex, uint32 FirstInstance,
                     uint32 NumVertices, uint32 StartIndex, uint32 NumPrimitives, uint32 NumInstances);

    /// @brief Copy the content of a buffer to another buffer
    ///
    /// @param Source The buffer to copy from
    /// @param Destination The buffer to copy to
    /// @param SourceOffset The offset in the source buffer
    /// @param DestinationOffset The offset in the destination buffer
    /// @param Size The number of bytes to copy
    void CopyBufferToBuffer(const Ref<RRHIBuffer>& Source, Ref<RRHIBuffer>& Destination, uint64 SourceOffset,
                            uint64 DestinationOffset, uint64 Size);

    /// @brief Copy the content of a resource array to a buffer
    ///
    /// @param Source The resource array to copy from (the caller is responsible for the lifetime of the object)
    /// @param Destination The buffer to copy to
    /// @param SourceOffset The offset in the source buffer
    /// @param DestinationOffset The offset in the destination buffer
    /// @param Size The number of bytes to copy
    void CopyRessourceArrayToBuffer(IResourceArrayInterface* Source, Ref<RRHIBuffer>& Destination, uint64 SourceOffset,
                                    uint64 DestinationOffset, uint64 Size);

    /// @brief Add a command to the back of the queue
    template <typename TSTR, typename TFunction, typename... ArgsType>
    requires std::is_invocable_v<TFunction, FFRHICommandList&, ArgsType...>
    void EnqueueLambda(TFunction&& Function, ArgsType&&... Args)
    {
        return Enqueue(new TLambdaRenderCommandType<TSTR, TFunction, ArgsType...>(std::forward<TFunction>(Function),
                                                                                  std::forward<ArgsType>(Args)...));
    }

    /// @brief Execute the command in the given context
    void Execute(FRHIContext* const Context);

    FRHIContext* GetContext() const
    {
        return m_Context;
    }

private:
    /// @brief Add a command to the back of the queue
    /// @note Take ownership of the command
    void Enqueue(FRHIRenderCommandBase* RenderCommand);

    /// @brief Reset the command list, deleting all the commands
    void Reset();

private:
    FRHIContext* m_Context = nullptr;

    bool bIsExecuting = false;
    /// @brief Head of the command queue
    /// @note "this" has ownership of the object. Can't use unique_ptr because of some ""issues"" with destroying the
    /// list without stack overflow
    FRHIRenderCommandBase* m_CommandList = nullptr;

    /// @brief Tail of the command queue
    FRHIRenderCommandBase* m_CommandListTail = nullptr;
};

class FRHICommandListExecutor
{
public:
    static FRHICommandListExecutor& Get()
    {
        static FRHICommandListExecutor Instance;
        return Instance;
    }

    static FFRHICommandList& GetCommandList()
    {
        return Get().m_CommandList;
    }

private:
    FFRHICommandList m_CommandList;
};

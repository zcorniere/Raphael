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

private:
    int m_Dummy[1'000] = {0};
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
    void BeginFrame();
    void EndFrame();

    void BeginRenderingViewport(RHIViewport* Viewport);
    void EndRenderingViewport(RHIViewport* Viewport, bool bPresent);

    void BeginRendering(const RHIRenderPassDescription& Description);
    void EndRendering();

    void TmpDraw(Ref<RHIGraphicsPipeline>& Pipeline);

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
extern RHICommandListExecutor GCommandListExecutor;

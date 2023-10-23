#pragma once

#include "Engine/Core/RHI/RHI.hxx"

#define ENQUEUE_RENDER_COMMAND(Type)                  \
    struct Type##Name {                               \
        static constexpr const std::string_view Str() \
        {                                             \
            return std::string_view(#Type);           \
        }                                             \
    };                                                \
    RHI::GetRHICommandQueue()->EnqueueCommand<Type##Name>

DECLARE_LOGGER_CATEGORY(Core, LogRenderCommand, Warning)

/// Dummy class to represent a RenderCommand Lambda
class RenderCommand
{
public:
    virtual void DoTask() = 0;
};

template <typename TSTR, typename LAMBDA>
requires std::invocable<LAMBDA> && requires {
    {
        TSTR::Str()
    } -> std::convertible_to<std::string_view>;
}
class TUniqueRenderCommandType : public RenderCommand
{
public:
    TUniqueRenderCommandType(LAMBDA&& InLambda): Lambda(std::forward<LAMBDA>(InLambda))
    {
    }
    virtual ~TUniqueRenderCommandType()
    {
    }

    virtual void DoTask() override
    {
        RPH_PROFILE_FUNC();
        LOG(LogRenderCommand, Trace, "Running task: {:s}", TSTR::Str());
        Lambda();
    }

private:
    LAMBDA Lambda;
};

/// @brief This class enqueue commands in a buffer
///
/// @code{.cpp}
/// Ref<RHICommandQueue> Queue = Ref<RHICommandQueue>::Create();
///
/// Queue.EnqueueCommand([] {
///     // Execution code
/// });
/// // Flush the queue and execute the function
/// Queue.Execute();
/// @endcode
class RHICommandQueue : public RObject
{
private:
    constexpr static auto AllocatedSize = 10 * 1024 * 1024;    // 10mb buffer

    using RenderCommandFn = void (*)(void*);

public:
    RHICommandQueue();
    ~RHICommandQueue();

    /// @brief Add a command to the back of the queue
    template <typename TSTR, typename TFunction>
    void EnqueueCommand(TFunction&& Function)
    {
        using TUniqueRenderCommand = TUniqueRenderCommandType<TSTR, TFunction>;
        RenderCommandFn RenderCmd = [](void* ptr) -> void {
            TUniqueRenderCommand* pFunction = (TUniqueRenderCommand*)ptr;
            pFunction->DoTask();

            pFunction->~TUniqueRenderCommand();
        };

        // If we are currently executing that command queue, run the task immediatly
        if (this->IsCurrentlyExecuting) {
            TUniqueRenderCommand Command(std::forward<TFunction>(Function));
            Command.DoTask();
        } else {
            void* pStorageBuffer = this->Allocate(RenderCmd, sizeof(TUniqueRenderCommandType<TSTR, TFunction>));
            new (pStorageBuffer) TUniqueRenderCommandType<TSTR, TFunction>(std::forward<TFunction>(Function));
        }
    }

    void Execute();

private:
    void* Allocate(RenderCommandFn Func, uint32_t Size);

private:
    uint8* m_CommandBuffer;
    uint8* m_CommandBufferCursor;
    uint32 m_CommandCount;

    const uint8* m_CommandBufferUpperLimit;
    bool IsCurrentlyExecuting = false;
};

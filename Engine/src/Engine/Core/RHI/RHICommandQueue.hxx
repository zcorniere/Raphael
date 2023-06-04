#pragma once

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
public:
    constexpr static auto AllocatedSize = 10 * 1024 * 1024;    // 10mb buffer

    using RenderCommandFn = void (*)(void*);

public:
    RHICommandQueue();
    ~RHICommandQueue();

    template <typename TFunction>
    void EnqueueCommand(TFunction&& Function)
    {
        RenderCommandFn RenderCmd = [](void* ptr) {
            TFunction* pFunction = (TFunction*)ptr;
            (*pFunction)();

            pFunction->~TFunction();
        };

        void* pStorageBuffer = this->Allocate(RenderCmd, sizeof(Function));
        new (pStorageBuffer) TFunction(std::forward<TFunction>(Function));
    }

    void* Allocate(RenderCommandFn Func, uint32_t Size);
    void Execute();

private:
    uint8* m_CommandBuffer;
    uint8* m_CommandBufferCursor;
    uint32 m_CommandCount;

    const uint8* m_CommandBufferUpperLimit;
};

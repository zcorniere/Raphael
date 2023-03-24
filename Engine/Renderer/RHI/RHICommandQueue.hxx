#pragma once

class RHICommandQueue
{
public:
    constexpr static auto AllocatedSize = 10 * 1024 * 1024;    // 10mb buffer
    
    using RenderCommandFn = void (*)(void *);

    RHICommandQueue();
    ~RHICommandQueue();

    void *Allocate(RenderCommandFn Func, uint32_t Size);
    void Execute();

private:
    uint8 *m_CommandBuffer;
    uint8 *m_CommandBufferCursor;
    uint32 m_CommandCount;
};

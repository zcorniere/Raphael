#include "Engine/Renderer/RHI/RHICommandQueue.hxx"

RHICommandQueue::RHICommandQueue()
{
    m_CommandBuffer = new uint8[AllocatedSize];
    m_CommandBufferCursor = m_CommandBuffer;
    std::memset(m_CommandBuffer, 0, AllocatedSize);
}

RHICommandQueue::~RHICommandQueue()
{
    delete[] m_CommandBuffer;
}

void *RHICommandQueue::Allocate(RenderCommandFn Func, uint32_t Size)
{
    *(RenderCommandFn *)m_CommandBufferCursor = Func;
    m_CommandBufferCursor += sizeof(RenderCommandFn);

    *(uint32 *)m_CommandBufferCursor = Size;
    m_CommandBufferCursor += sizeof(uint32);

    void *Memory = m_CommandBufferCursor;
    m_CommandBufferCursor += Size;

    m_CommandCount += 1;
    return Memory;
}

void RHICommandQueue::Execute()
{
    uint8 *Buffer = m_CommandBuffer;

    for (uint32_t i = 0; i < m_CommandCount; i++) {
        RenderCommandFn Function = *(RenderCommandFn *)Buffer;
        Buffer += sizeof(RenderCommandFn);

        uint32 Size = *(uint32 *)Buffer;
        Buffer += sizeof(uint32);
        Function(Buffer);
        Buffer += Size;
    }

    m_CommandBufferCursor = m_CommandBuffer;
    m_CommandCount = 0;
}

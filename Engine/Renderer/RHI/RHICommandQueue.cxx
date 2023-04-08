#include "Engine/Renderer/RHI/RHICommandQueue.hxx"

RHICommandQueue::RHICommandQueue(): m_CommandCount(0)
{
    m_CommandBuffer = new uint8[AllocatedSize];
    m_CommandBufferUpperLimit = m_CommandBuffer + AllocatedSize;

    m_CommandBufferCursor = m_CommandBuffer;
    std::memset(m_CommandBuffer, 0, AllocatedSize);
}

RHICommandQueue::~RHICommandQueue()
{
    delete[] m_CommandBuffer;
}

void *RHICommandQueue::Allocate(RenderCommandFn Func, uint32_t Size)
{
    verifyAlwaysMsg(Size > 0, "RenderCommand has a size of 0 !");

    *(RenderCommandFn *)m_CommandBufferCursor = Func;
    m_CommandBufferCursor += sizeof(RenderCommandFn);
    checkMsg(m_CommandBufferCursor < m_CommandBufferUpperLimit,
             "RHICommandQueue has overflowed with {} registered command !", m_CommandCount);

    *(uint32 *)m_CommandBufferCursor = Size;
    m_CommandBufferCursor += sizeof(uint32);
    checkMsg(m_CommandBufferCursor < m_CommandBufferUpperLimit,
             "RHICommandQueue has overflowed with {} registered command !", m_CommandCount);

    void *Memory = m_CommandBufferCursor;
    m_CommandBufferCursor += Size;
    checkMsg(m_CommandBufferCursor < m_CommandBufferUpperLimit,
             "RHICommandQueue has overflowed with {} registered command !", m_CommandCount);

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

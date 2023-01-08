#include "Engine/Renderer/RenderCommandQueue.hxx"

namespace Raphael
{

RenderCommandQueue::RenderCommandQueue(const std::uint32_t Size)
{
    p_CommandBuffer = new std::byte[Size];
    p_CommandBufferCursor = p_CommandBuffer;
    m_NumberOfCommand = 0;
}

RenderCommandQueue::~RenderCommandQueue()
{
    delete[] p_CommandBuffer;
}

void *RenderCommandQueue::Allocate(RenderCommandQueue::CommandBufferFunction Fn, std::uint32_t SizeOfArgs)
{
    // TODO: alignment
    *(CommandBufferFunction *)p_CommandBufferCursor = Fn;
    p_CommandBufferCursor += sizeof(CommandBufferFunction);

    *(std::uint32_t *)p_CommandBufferCursor = SizeOfArgs;
    p_CommandBufferCursor += sizeof(std::uint32_t);

    void *memory = p_CommandBufferCursor;
    p_CommandBufferCursor += SizeOfArgs;

    m_NumberOfCommand++;
    return memory;
}

void RenderCommandQueue::Execute()
{
    std::byte *Cursor = p_CommandBuffer;

    for (std::uint32_t i = 0; i < m_NumberOfCommand; i++) {
        CommandBufferFunction function = *(CommandBufferFunction *)Cursor;
        Cursor += sizeof(CommandBufferFunction);

        std::uint32_t size = *(std::uint32_t *)Cursor;
        Cursor += sizeof(std::uint32_t);
        function(Cursor);
        Cursor += size;
    }

    p_CommandBufferCursor = p_CommandBuffer;
    m_NumberOfCommand = 0;
}

}    // namespace Raphael

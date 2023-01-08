#pragma once

namespace Raphael
{

class RenderCommandQueue
{
public:
    using CommandBufferFunction = void (*)(void *);

public:
    RenderCommandQueue(const std::uint32_t CommandBufferSize = 1024 * 1024 * 10);    // 10mb by default;
    ~RenderCommandQueue();

    void *Allocate(CommandBufferFunction Fn, std::uint32_t SizeOfArgs);
    void Execute();

private:
    std::byte *p_CommandBuffer = nullptr;
    std::byte *p_CommandBufferCursor = nullptr;
    std::uint32_t m_NumberOfCommand = 0;
};

}    // namespace Raphael

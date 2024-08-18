#pragma once

#include "Engine/Core/RHI/RHICommandList.hxx"

#define RHICOMMAND_MACRO(CommandName)      \
    struct CommandName##String {           \
        static constexpr const char* Str() \
        {                                  \
            return #CommandName;           \
        }                                  \
    };                                     \
    class CommandName final : public TRHIRenderCommand<CommandName##String>

RHICOMMAND_MACRO(RHIBeginFrame)
{
public:
    RHIBeginFrame() = default;
    virtual ~RHIBeginFrame() = default;

    virtual void Execute(RHICommandList & CommandList) override final;
};

RHICOMMAND_MACRO(RHIEndFrame)
{
public:
    RHIEndFrame() = default;
    virtual ~RHIEndFrame() = default;

    virtual void Execute(RHICommandList & CommandList) override final;
};

RHICOMMAND_MACRO(RHIBeginDrawingViewport)
{
public:
    RHIBeginDrawingViewport() = delete;
    RHIBeginDrawingViewport(Ref<RHIViewport> InViewport);
    virtual ~RHIBeginDrawingViewport() = default;

    virtual void Execute(RHICommandList & CommandList) override final;

public:
    Ref<RHIViewport> Viewport = nullptr;
};

RHICOMMAND_MACRO(RHIEndDrawningViewport)
{
public:
    RHIEndDrawningViewport() = delete;
    RHIEndDrawningViewport(Ref<RHIViewport> InViewport, bool bInPresent);
    virtual ~RHIEndDrawningViewport() = default;

    virtual void Execute(RHICommandList & CommandList) override final;

public:
    Ref<RHIViewport> Viewport = nullptr;
    const bool bPresent = false;
};

RHICOMMAND_MACRO(RHIBeginRendering)
{
public:
    RHIBeginRendering(const RHIRenderPassDescription& InDescription);
    virtual ~RHIBeginRendering() = default;

    virtual void Execute(RHICommandList & CommandList) override final;

public:
    const RHIRenderPassDescription Description = {};
};

RHICOMMAND_MACRO(RHIEndRendering)
{
public:
    RHIEndRendering() = default;
    virtual ~RHIEndRendering() = default;

    virtual void Execute(RHICommandList & CommandList) override final;
};

RHICOMMAND_MACRO(RHISetPipeline)
{
public:
    RHISetPipeline(Ref<RHIGraphicsPipeline> InPipeline);
    virtual ~RHISetPipeline() = default;

    virtual void Execute(RHICommandList & CommandList) override final;

private:
    Ref<RHIGraphicsPipeline> Pipeline = nullptr;
};

RHICOMMAND_MACRO(RHISetVertexBuffer)
{
public:
    RHISetVertexBuffer(Ref<RHIBuffer> InVertexBuffer, uint32 BufferIndex = 0, uint32 Offset = 0);
    virtual ~RHISetVertexBuffer() = default;

    virtual void Execute(RHICommandList & CommandList) override final;

private:
    Ref<RHIBuffer> VertexBuffer = nullptr;
    uint32 BufferIndex = 0;
    uint32 Offset = 0;
};

RHICOMMAND_MACRO(RHISetViewport)
{
public:
    RHISetViewport(FVector3 Min, FVector3 Max);
    virtual ~RHISetViewport() = default;

    virtual void Execute(RHICommandList & CommandList) override final;

private:
    FVector3 Min;
    FVector3 Max;
};

RHICOMMAND_MACRO(RHISetScissor)
{
public:
    RHISetScissor(IVector2 Offset, UVector2 Size);
    virtual ~RHISetScissor() = default;

    virtual void Execute(RHICommandList & CommandList) override final;

private:
    IVector2 Offset;
    UVector2 Size;
};

RHICOMMAND_MACRO(RHIDraw)
{
public:
    RHIDraw(uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances);
    virtual ~RHIDraw() = default;

    virtual void Execute(RHICommandList & CommandList) override final;

public:
    uint32 BaseVertexIndex = 0;
    uint32 NumPrimitives = 0;
    uint32 NumInstances = 0;
};

RHICOMMAND_MACRO(RHICopyBufferToBuffer)
{
public:
    RHICopyBufferToBuffer(const Ref<RHIBuffer> Source, Ref<RHIBuffer> Destination, uint64 SourceOffset,
                          uint64 DestinationOffset, uint64 Size);
    virtual ~RHICopyBufferToBuffer() = default;

    virtual void Execute(RHICommandList & CommandList) override final;

private:
    const Ref<RHIBuffer> SourceBuffer = nullptr;
    Ref<RHIBuffer> DestinationBuffer = nullptr;
    uint64 Size = 0;
    uint64 SourceOffset = 0;
    uint64 DestinationOffset = 0;
};

#undef RHICOMMAND_MACRO

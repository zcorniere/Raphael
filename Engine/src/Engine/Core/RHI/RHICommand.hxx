#pragma once

#include "Engine/Core/RHI/RHICommandList.hxx"

#define RHICOMMAND_MACRO(CommandName)      \
    struct CommandName##String             \
    {                                      \
        static constexpr const char* Str() \
        {                                  \
            return #CommandName;           \
        }                                  \
    };                                     \
    class CommandName final : public TRHIRenderCommand<CommandName##String>

RHICOMMAND_MACRO(FRHIBeginFrame)
{
public:
    FRHIBeginFrame() = default;
    virtual ~FRHIBeginFrame() = default;

    virtual void Execute(FFRHICommandList & CommandList) override final;
};

RHICOMMAND_MACRO(FRHIEndFrame)
{
public:
    FRHIEndFrame() = default;
    virtual ~FRHIEndFrame() = default;

    virtual void Execute(FFRHICommandList & CommandList) override final;
};

RHICOMMAND_MACRO(FRHIBeginDrawingViewport)
{
public:
    FRHIBeginDrawingViewport() = delete;
    FRHIBeginDrawingViewport(Ref<RRHIViewport> InViewport);
    virtual ~FRHIBeginDrawingViewport() = default;

    virtual void Execute(FFRHICommandList & CommandList) override final;

public:
    Ref<RRHIViewport> Viewport = nullptr;
};

RHICOMMAND_MACRO(FRHIEndDrawningViewport)
{
public:
    FRHIEndDrawningViewport() = delete;
    FRHIEndDrawningViewport(Ref<RRHIViewport> InViewport);
    virtual ~FRHIEndDrawningViewport() = default;

    virtual void Execute(FFRHICommandList & CommandList) override final;

public:
    Ref<RRHIViewport> Viewport = nullptr;
};

RHICOMMAND_MACRO(FRHIBeginRendering)
{
public:
    FRHIBeginRendering(const FRHIRenderPassDescription& InDescription);
    virtual ~FRHIBeginRendering() = default;

    virtual void Execute(FFRHICommandList & CommandList) override final;

public:
    const FRHIRenderPassDescription Description = {};
};

RHICOMMAND_MACRO(FRHIEndRendering)
{
public:
    FRHIEndRendering() = default;
    virtual ~FRHIEndRendering() = default;

    virtual void Execute(FFRHICommandList & CommandList) override final;
};

RHICOMMAND_MACRO(FRHISetMaterial)
{
public:
    FRHISetMaterial(Ref<RRHIMaterial> InMaterial);
    virtual ~FRHISetMaterial() = default;

    virtual void Execute(FFRHICommandList & CommandList) override final;

private:
    Ref<RRHIMaterial> Material = nullptr;
};

RHICOMMAND_MACRO(FRHISetVertexBuffer)
{
public:
    FRHISetVertexBuffer(Ref<RRHIBuffer> InVertexBuffer, uint32 BufferIndex = 0, uint32 Offset = 0);
    virtual ~FRHISetVertexBuffer() = default;

    virtual void Execute(FFRHICommandList & CommandList) override final;

private:
    Ref<RRHIBuffer> VertexBuffer = nullptr;
    uint32 BufferIndex = 0;
    uint32 Offset = 0;
};

RHICOMMAND_MACRO(FRHISetViewport)
{
public:
    FRHISetViewport(FVector3 Min, FVector3 Max);
    virtual ~FRHISetViewport() = default;

    virtual void Execute(FFRHICommandList & CommandList) override final;

private:
    FVector3 Min;
    FVector3 Max;
};

RHICOMMAND_MACRO(FRHISetScissor)
{
public:
    FRHISetScissor(IVector2 Offset, UVector2 Size);
    virtual ~FRHISetScissor() = default;

    virtual void Execute(FFRHICommandList & CommandList) override final;

private:
    IVector2 Offset;
    UVector2 Size;
};

RHICOMMAND_MACRO(FRHIDraw)
{
public:
    FRHIDraw(uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances);
    virtual ~FRHIDraw() = default;

    virtual void Execute(FFRHICommandList & CommandList) override final;

public:
    uint32 BaseVertexIndex = 0;
    uint32 NumPrimitives = 0;
    uint32 NumInstances = 0;
};

RHICOMMAND_MACRO(RHIDrawIndexed)
{
public:
    RHIDrawIndexed(Ref<RRHIBuffer> InIndexBuffer, int32 BaseVertexIndex, uint32 FirstInstance, uint32 NumVertices,
                   uint32 StartIndex, uint32 NumPrimitives, uint32 NumInstances);
    virtual ~RHIDrawIndexed() = default;

    virtual void Execute(FFRHICommandList & CommandList) override final;

private:
    Ref<RRHIBuffer> IndexBuffer = nullptr;
    int32 BaseVertexIndex = 0;
    uint32 FirstInstance = 0;
    uint32 NumVertices = 0;
    uint32 StartIndex = 0;
    uint32 NumPrimitives = 0;
    uint32 NumInstances = 0;
};

RHICOMMAND_MACRO(RHICopyResourceArrayToBuffer)
{
public:
    RHICopyResourceArrayToBuffer(IResourceArrayInterface* const Source, Ref<RRHIBuffer> Destination,
                                 uint64 SourceOffset, uint64 DestinationOffset, uint64 Size);
    virtual ~RHICopyResourceArrayToBuffer() = default;

    virtual void Execute(FFRHICommandList & CommandList) override final;

private:
    IResourceArrayInterface* const SourceArray = nullptr;
    Ref<RRHIBuffer> DestinationBuffer = nullptr;
    uint64 Size = 0;
    uint64 SourceOffset = 0;
    uint64 DestinationOffset = 0;
};

RHICOMMAND_MACRO(RHICopyBufferToBuffer)
{
public:
    RHICopyBufferToBuffer(const Ref<RRHIBuffer> Source, Ref<RRHIBuffer> Destination, uint64 SourceOffset,
                          uint64 DestinationOffset, uint64 Size);
    virtual ~RHICopyBufferToBuffer() = default;

    virtual void Execute(FFRHICommandList & CommandList) override final;

private:
    const Ref<RRHIBuffer> SourceBuffer = nullptr;
    Ref<RRHIBuffer> DestinationBuffer = nullptr;
    uint64 Size = 0;
    uint64 SourceOffset = 0;
    uint64 DestinationOffset = 0;
};

#undef RHICOMMAND_MACRO

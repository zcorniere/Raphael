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
    RHIBeginDrawingViewport(WeakRef<RHIViewport> InViewport);
    virtual ~RHIBeginDrawingViewport() = default;

    virtual void Execute(RHICommandList & CommandList) override final;

public:
    WeakRef<RHIViewport> Viewport = nullptr;
};

RHICOMMAND_MACRO(RHIEndDrawningViewport)
{
public:
    RHIEndDrawningViewport() = delete;
    RHIEndDrawningViewport(WeakRef<RHIViewport> InViewport, bool bInPresent);
    virtual ~RHIEndDrawningViewport() = default;

    virtual void Execute(RHICommandList & CommandList) override final;

public:
    WeakRef<RHIViewport> Viewport = nullptr;
    bool bPresent = false;
};

RHICOMMAND_MACRO(RHIResizeViewport)
{
public:
    RHIResizeViewport() = delete;
    RHIResizeViewport(WeakRef<RHIViewport> InViewport, uint32_t InWidth, uint32_t InHeight);
    virtual ~RHIResizeViewport() = default;

    virtual void Execute(RHICommandList & CommandList) override;

public:
    WeakRef<RHIViewport> Viewport = nullptr;
    uint32_t Width = 0;
    uint32_t Height = 0;
};

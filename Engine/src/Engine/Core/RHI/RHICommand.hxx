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

RHICOMMAND_MACRO(RHIResizeViewport)
{
public:
    RHIResizeViewport() = delete;
    RHIResizeViewport(Ref<RHIViewport> InViewport, uint32_t InWidth, uint32_t InHeight);
    virtual ~RHIResizeViewport() = default;

    virtual void Execute(RHICommandList & CommandList) override;

public:
    Ref<RHIViewport> Viewport = nullptr;
    const uint32_t Width = 0;
    const uint32_t Height = 0;
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

#pragma once

#include "Engine/Renderer/RendererCapabilities.hxx"

#include "Engine/Core/Application.hxx"

namespace Raphael
{

class Renderer
{
public:
    DECLARE_LOGGER_CATEGORY(Core, LogRenderer, Info);

public:
    static Ref<RendererContext> GetContext()
    {
        return Application::Get().GetWindow().GetRenderContext();
    }

    typedef void (*RenderCommandFn)(void *);

    static void Init();
    static void Shutdown();

    static void SetConfig(const RendererConfig &config);

    static const RendererCapabilities &GetCapabilities();
};

namespace Utils
{
    inline void DumpGPUInfo()
    {
        auto &caps = Renderer::GetCapabilities();
        LOG(Renderer::LogRenderer, Info, "GPU Info:");
        LOG(Renderer::LogRenderer, Info, "  Vendor: {0}", caps.Vendor);
        LOG(Renderer::LogRenderer, Info, "  Device: {0}", caps.Device);
        LOG(Renderer::LogRenderer, Info, "  Version: {0}", caps.Version);
    }

}    // namespace Utils

}    // namespace Raphael

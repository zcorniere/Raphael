#pragma once

#include "Engine/Renderer/RendererCapabilities.hxx"
#include "Engine/Renderer/RendererConfig.hxx"

namespace Raphael
{

class Renderer
{
public:
    typedef void (*RenderCommandFn)(void *);

    static void Init();
    static void Shutdown();

    static void SetConfig(const RendererConfig &config);

    static RendererCapabilities &GetCapabilities();
};

}    // namespace Raphael

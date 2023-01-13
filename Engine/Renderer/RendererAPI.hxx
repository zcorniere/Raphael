#pragma once

#include "Engine/Renderer/RendererCapabilities.hxx"

namespace Raphael
{

enum class RendererAPIType {
    None,
    Vulkan,
};

class RendererAPI
{
public:
    static RendererAPIType Current()
    {
        return s_CurrentRendererAPI;
    }
    static void SetAPI(RendererAPIType api);

public:
    virtual void Init() = 0;
    virtual void Shutdown() = 0;

    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;

    virtual const RendererCapabilities &GetCapabilities() const = 0;

private:
    inline static RendererAPIType s_CurrentRendererAPI = RendererAPIType::Vulkan;
};

}    // namespace Raphael

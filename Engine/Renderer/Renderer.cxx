#include "Engine/Renderer/Renderer.hxx"
#include "Engine/Renderer/RendererAPI.hxx"
#include "Engine/Renderer/RendererConfig.hxx"

#include "Engine/Renderer/Vulkan/VulkanRenderer.hxx"

namespace Raphael
{

void RendererAPI::SetAPI(RendererAPIType api)
{
    checkMsg(api == RendererAPIType::Vulkan, "Vulkan is the only supported renderer form now");
    s_CurrentRendererAPI = api;
}

static RendererConfig s_Config;
static RendererAPI *s_RendererAPI = nullptr;

static RendererAPI *InitRendererAPI()
{
    switch (RendererAPI::Current()) {
        case RendererAPIType::Vulkan: return new VulkanRenderer();
        default: checkMsg(false, "Unknowned Renderer !"); return nullptr;
    }
    checkNoEntry();
}

void Renderer::Init()
{
    s_RendererAPI = InitRendererAPI();

    s_RendererAPI->Init();
}

void Renderer::Shutdown()
{
    s_RendererAPI->Shutdown();
}

void Renderer::SetConfig(const RendererConfig &config)
{
    s_Config = config;
}

RendererConfig &Renderer::GetConfig()
{
    return s_Config;
}

const RendererCapabilities &Renderer::GetCapabilities()
{
    return s_RendererAPI->GetCapabilities();
}

}    // namespace Raphael

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
static RenderCommandQueue *s_CommandQueue = nullptr;
static RenderCommandQueue *s_ResourceFreeQueue = nullptr;

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

    s_ResourceFreeQueue = new RenderCommandQueue[s_Config.FramesInFlight];
    s_CommandQueue = new RenderCommandQueue();

    s_RendererAPI->Init();
}

void Renderer::Shutdown()
{
    s_RendererAPI->Shutdown();

    delete s_CommandQueue;
    delete[] s_ResourceFreeQueue;
}

void Renderer::BeginFrame()
{
    return s_RendererAPI->BeginFrame();
}

void Renderer::EndFrame()
{
    return s_RendererAPI->EndFrame();
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

std::uint32_t Renderer::GetCurrentSwapchainIndex()
{
    return Application::Get().GetWindow().GetSwapChain().GetCurrentBufferIndex();
}

RenderCommandQueue &Renderer::GetRenderResourceReleaseQueue(uint32_t index)
{
    return s_ResourceFreeQueue[index];
}

RenderCommandQueue &Renderer::GetRenderCommandQueue()
{
    return *s_CommandQueue;
}

void Renderer::WaitAndRender()
{
    s_CommandQueue->Execute();
}

}    // namespace Raphael

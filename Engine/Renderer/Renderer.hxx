#pragma once

#include "Engine/Renderer/RenderCommandQueue.hxx"
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

    static void Init();
    static void Shutdown();

    static void SetConfig(const RendererConfig &config);
    static RendererConfig &GetConfig();

    static void BeginFrame();
    static void EndFrame();

    static const RendererCapabilities &GetCapabilities();
    static std::uint32_t GetCurrentSwapchainIndex();

private:
    static RenderCommandQueue &GetRenderResourceReleaseQueue(uint32_t index);
    static RenderCommandQueue &GetRenderCommandQueue();

public:
    template <typename FuncT>
    static void Submit(FuncT &&func)
    {
        auto renderCmd = [](void *ptr) {
            auto pFunc = (FuncT *)ptr;
            (*pFunc)();
            pFunc->~FuncT();
        };
        auto storageBuffer = GetRenderCommandQueue().Allocate(renderCmd, sizeof(func));
        new (storageBuffer) FuncT(std::forward<FuncT>(func));
    }

    template <typename FuncT>
    static void SubmitResourceFree(FuncT &&func)
    {
        auto renderCmd = [](void *ptr) {
            auto pFunc = (FuncT *)ptr;
            (*pFunc)();
            pFunc->~FuncT();
        };

        Submit([renderCmd, func]() {
            const uint32_t index = Renderer::GetCurrentSwapchainIndex();
            auto storageBuffer = GetRenderResourceReleaseQueue(index).Allocate(renderCmd, sizeof(func));
            new (storageBuffer) FuncT(std::forward<FuncT>((FuncT &&) func));
        });
    }

    static void WaitAndRender();
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

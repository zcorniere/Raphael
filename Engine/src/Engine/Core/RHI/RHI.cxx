#include "Engine/Core/RHI/RHI.hxx"

#include "Engine/Core/RHI/GenericRHI.hxx"
#include "Engine/Core/RHI/RHICommandQueue.hxx"
#include "Engine/Core/Window.hxx"

static RHICommandQueue* s_CommandQueue = nullptr;

GenericRHI* GDynamicRHI = nullptr;

void RHI::Init()
{
    s_CommandQueue = new RHICommandQueue;
}

void RHI::DeleteRHI()
{
    GDynamicRHI->Shutdown();

    delete s_CommandQueue;
    s_CommandQueue = nullptr;

    delete GDynamicRHI;
    GDynamicRHI = nullptr;
}

RHICommandQueue* RHI::GetRHICommandQueue()
{
    check(s_CommandQueue);
    return s_CommandQueue;
}

/// RHI Fowarding
void RHI::BeginFrame()
{
    return RHI::Get()->BeginFrame();
}

void RHI::EndFrame()
{
    return RHI::Get()->EndFrame();
}

void RHI::NextFrame()
{
    return RHI::Get()->NextFrame();
}

void RHI::BeginRenderPass(const RHIRenderPassDescription& Renderpass, const RHIFramebufferDefinition& Framebuffer)
{
    return RHI::Get()->BeginRenderPass(Renderpass, Framebuffer);
}

void RHI::EndRenderPass()
{
    return RHI::Get()->EndRenderPass();
}

void RHI::Draw(Ref<RHIGraphicsPipeline>& Pipeline)
{
    return RHI::Get()->Draw(Pipeline);
}

//
//  -------------------- RHI Create resources --------------------
//

Ref<RHIViewport> RHI::CreateViewport(Ref<Window> InWindowHandle, glm::uvec2 InSize)
{
    return RHI::Get()->CreateViewport(std::move(InWindowHandle), std::move(InSize));
}

Ref<RHITexture> RHI::CreateTexture(const RHITextureSpecification& InDesc)
{
    return RHI::Get()->CreateTexture(InDesc);
}

Ref<RHIBuffer> RHI::CreateBuffer(const RHIBufferDesc& InDesc)
{
    return RHI::Get()->CreateBuffer(InDesc);
}

Ref<RHIShader> RHI::CreateShader(const std::filesystem::path Path, bool bForceCompile)
{
    return RHI::Get()->CreateShader(Path, bForceCompile);
}

Ref<RHIGraphicsPipeline> RHI::CreateGraphicsPipeline(const RHIGraphicsPipelineSpecification& Config)
{
    return RHI::Get()->CreateGraphicsPipeline(Config);
}

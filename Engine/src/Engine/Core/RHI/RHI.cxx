#include "Engine/Core/RHI/RHI.hxx"

#include "Engine/Core/RHI/GenericRHI.hxx"
#include "Engine/Core/Window.hxx"
#include "RHI.hxx"

static std::unique_ptr<RHICommandQueue> s_CommandQueue = nullptr;

std::unique_ptr<GenericRHI> GDynamicRHI = nullptr;

void RHI::Init()
{
    s_CommandQueue = std::make_unique<RHICommandQueue>();
}

void RHI::DeleteRHI()
{
    GDynamicRHI->Shutdown();
    s_CommandQueue = nullptr;
    GDynamicRHI = nullptr;
}

RHICommandQueue* RHI::GetRHICommandQueue()
{
    check(s_CommandQueue);
    return s_CommandQueue.get();
}

/// RHI Fowarding
void RHI::BeginFrame()
{
    RPH_PROFILE_FUNC()

    return RHI::Get<GenericRHI>()->BeginFrame();
}

void RHI::EndFrame()
{
    RPH_PROFILE_FUNC()

    return RHI::Get<GenericRHI>()->EndFrame();
}

void RHI::NextFrame()
{
    RPH_PROFILE_FUNC()

    return RHI::Get<GenericRHI>()->NextFrame();
}

void RHI::BeginRenderPass(const RHIRenderPassDescription& Renderpass, const RHIFramebufferDefinition& Framebuffer)
{
    RPH_PROFILE_FUNC()

    return RHI::Get<GenericRHI>()->BeginRenderPass(Renderpass, Framebuffer);
}

void RHI::EndRenderPass()
{
    RPH_PROFILE_FUNC()
    
    return RHI::Get<GenericRHI>()->EndRenderPass();
}

void RHI::Draw(Ref<RHIGraphicsPipeline>& Pipeline)
{
    return RHI::Get<GenericRHI>()->Draw(Pipeline);
}
//
//  -------------------- RHI Create resources --------------------
//

Ref<RHIViewport> RHI::CreateViewport(Ref<Window> InWindowHandle, glm::uvec2 InSize)
{
    return RHI::Get<GenericRHI>()->CreateViewport(std::move(InWindowHandle), std::move(InSize));
}

Ref<RHITexture> RHI::CreateTexture(const RHITextureSpecification& InDesc)
{
    return RHI::Get<GenericRHI>()->CreateTexture(InDesc);
}

Ref<RHIBuffer> RHI::CreateBuffer(const uint32 InSize, const EBufferUsageFlags InUsage, const uint32 InStride,
                                 Ref<ResourceArray>& InitialData)
{
    return RHI::Get<GenericRHI>()->CreateBuffer(InSize, InUsage, InStride, InitialData);
}

Ref<RHIShader> RHI::CreateShader(const std::filesystem::path Path, bool bForceCompile)
{
    return RHI::Get<GenericRHI>()->CreateShader(Path, bForceCompile);
}

Ref<RHIGraphicsPipeline> RHI::CreateGraphicsPipeline(const RHIGraphicsPipelineSpecification& Config)
{
    return RHI::Get<GenericRHI>()->CreateGraphicsPipeline(Config);
}


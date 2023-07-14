#include "Engine/Core/RHI/RHI.hxx"

#include "Engine/Core/RHI/GenericRHI.hxx"
#include "RHI.hxx"

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

void RHI::BeginRenderPass(const RHIRenderPassDescription& Description)
{
    RPH_PROFILE_FUNC()

    return RHI::Get<GenericRHI>()->BeginRenderPass(Description);
}

void RHI::EndRenderPass()
{
    RPH_PROFILE_FUNC()
    
    return RHI::Get<GenericRHI>()->EndRenderPass();
}

//
//  -------------------- RHI Create resources --------------------
//

Ref<RHIViewport> RHI::CreateViewport(void* InWindowHandle, glm::uvec2 InSize)
{
    return RHI::Get<GenericRHI>()->CreateViewport(InWindowHandle, InSize);
}

Ref<RHITexture> RHI::CreateTexture(const RHITextureCreateDesc InDesc)
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

Ref<RHIGraphicsPipeline> RHI::CreateGraphicsPipeline(const RHIGraphicsPipelineInitializer& Config)
{
    return RHI::Get<GenericRHI>()->CreateGraphicsPipeline(Config);
}


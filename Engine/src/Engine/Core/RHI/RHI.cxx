#include "Engine/Core/RHI/RHI.hxx"

#include "Engine/Core/RHI/GenericRHI.hxx"

static Ref<RHICommandQueue> s_CommandQueue = nullptr;

Ref<GenericRHI> GDynamicRHI = nullptr;

void RHI::Init() { s_CommandQueue = Ref<RHICommandQueue>::Create(); }

void RHI::DeleteRHI()
{
    GDynamicRHI->Shutdown();
    s_CommandQueue = nullptr;
    GDynamicRHI = nullptr;
}

Ref<RHICommandQueue> &RHI::GetRHICommandQueue()
{
    check(s_CommandQueue);
    return s_CommandQueue;
}

/// RHI Fowarding
void RHI::BeginFrame() { return RHI::Get<GenericRHI>()->BeginFrame(); }

void RHI::EndFrame() { return RHI::Get<GenericRHI>()->EndFrame(); }

void RHI::NextFrame() { return RHI::Get<GenericRHI>()->NextFrame(); }

//
//  -------------------- RHI Create resources --------------------
//

Ref<RHIViewport> RHI::CreateViewport(void *InWindowHandle, glm::uvec2 InSize)
{
    return RHI::Get<GenericRHI>()->CreateViewport(InWindowHandle, InSize);
}

Ref<RHITexture> RHI::CreateTexture(const RHITextureCreateDesc InDesc)
{
    return RHI::Get<GenericRHI>()->CreateTexture(InDesc);
}

Ref<RHIShader> RHI::CreateShader(const std::filesystem::path Path, bool bForceCompile)
{
    return RHI::Get<GenericRHI>()->CreateShader(Path, bForceCompile);
}

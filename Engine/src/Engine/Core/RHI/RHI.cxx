#include "Engine/Core/RHI/RHI.hxx"

#include "Engine/Core/Engine.hxx"
#include "Engine/Core/RHI/GenericRHI.hxx"
#include "Engine/Core/RHI/RHICommandList.hxx"
#include "Engine/Core/RHI/RHIShaderParameters.hxx"
#include "Engine/Core/Window.hxx"

GenericRHI* GDynamicRHI = nullptr;

void RHI::Create()
{
    GDynamicRHI = RHI_CreateRHI();
}

void RHI::Destroy()
{
    GDynamicRHI->Shutdown();

    delete GDynamicRHI;
    GDynamicRHI = nullptr;
}

void RHI::BeginFrame()
{
}

void RHI::Tick(float fDeltaTime)
{
    return RHI::Get()->Tick(fDeltaTime);
}

void RHI::EndFrame()
{
    // Run the command list
    RHIContext* const Context = RHI::Get()->RHIGetCommandContext();
    RHICommandListExecutor::Get().GetCommandList().Execute(Context);
    RHI::Get()->RHIReleaseCommandContext(Context);

    GFrameCounter += 1;
}

void RHI::DeferedDeletion(std::function<void()>&& InDeletionFunction)
{
    RHI::Get()->DeferedDeletion(std::move(InDeletionFunction));
}

void RHI::FlushDeletionQueue()
{
    RHI::Get()->FlushDeletionQueue();
}

void RHI::RHIWaitUntilIdle()
{
    RHI::Get()->WaitUntilIdle();
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

std::future<Ref<RHIShader>> RHI::CreateShaderAsync(const std::filesystem::path Path, bool bForceCompile)
{
    return GEngine->GetThreadPool().Push(
        [Path, bForceCompile](int) { return RHI::Get()->CreateShader(Path, bForceCompile); });
}

Ref<RHIGraphicsPipeline> RHI::CreateGraphicsPipeline(const RHIGraphicsPipelineSpecification& Config)
{
    return RHI::Get()->CreateGraphicsPipeline(Config);
}

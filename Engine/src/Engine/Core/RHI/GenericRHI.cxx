#include "Engine/Core/RHI/GenericRHI.hxx"

void GenericRHI::Init() {}

void GenericRHI::PostInit() {}

void GenericRHI::Shutdown() {}

// RHI Operation default function
void GenericRHI::BeginFrame() { checkNoEntry(); }
void GenericRHI::EndFrame() { checkNoEntry(); }
void GenericRHI::NextFrame() { checkNoEntry(); }

Ref<RHIViewport> GenericRHI::CreateViewport(void *InWindowHandle, glm::uvec2 InSize)
{
    (void)InWindowHandle;
    (void)InSize;
    return nullptr;
}

Ref<RHITexture> GenericRHI::CreateTexture(const RHITextureCreateDesc InDesc)
{
    (void)InDesc;
    return nullptr;
}

Ref<RHIShader> GenericRHI::CreateShader(const std::filesystem::path Path, bool bForceCompile)
{
    (void)Path;
    (void)bForceCompile;
    return nullptr;
}

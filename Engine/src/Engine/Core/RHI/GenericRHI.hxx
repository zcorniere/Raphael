#pragma once

#include "Engine/Core/RHI/RHICommandQueue.hxx"
#include "Engine/Core/RHI/RHIResource.hxx"

#include "Engine/Core/RHI/RHI.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogGenericRHI, Info);

class GenericRHI : public RObject
{
public:
    virtual ~GenericRHI() {}

    // Initialize the RHI
    virtual void Init() = 0;

    // Called after the RHI init
    virtual void PostInit() = 0;

    // Shutdown the RHI
    virtual void Shutdown() = 0;

    virtual RHIInterfaceType GetInterfaceType() const { return RHIInterfaceType::Null; }

    // ---------------------- RHI Operations --------------------- //
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void NextFrame() = 0;

    virtual Ref<RHIViewport> CreateViewport(void *InWindowHandle, glm::uvec2 InSize) = 0;
    virtual Ref<RHITexture> CreateTexture(const RHITextureCreateDesc InDesc) = 0;
    virtual Ref<RHIShader> CreateShader(const std::filesystem::path Path, bool bForceCompile) = 0;

private:
    friend class RHI;
};

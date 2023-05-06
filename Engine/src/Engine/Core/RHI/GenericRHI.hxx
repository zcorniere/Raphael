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
    virtual void Init();

    // Called after the RHI init
    virtual void PostInit();

    // Shutdown the RHI
    virtual void Shutdown();

    virtual RHIInterfaceType GetInterfaceType() const { return RHIInterfaceType::Null; }

    // ---------------------- RHI Operations --------------------- //
    virtual void BeginFrame();
    virtual void EndFrame();
    virtual void NextFrame();

    virtual void BeginDrawingViewport(Ref<RHIViewport> &Viewport);
    virtual void EndDrawingViewport(Ref<RHIViewport> &Viewport);

    virtual Ref<RHIViewport> CreateViewport(void *InWindowHandle, glm::uvec2 InSize);
    virtual Ref<RHITexture> CreateTexture(const RHITextureCreateDesc InDesc);
    virtual Ref<RHIShader> CreateShader(const std::filesystem::path Path, bool bForceCompile);

private:
    friend class RHI;
};

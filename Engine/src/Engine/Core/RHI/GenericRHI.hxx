#pragma once

#include "Engine/Core/RHI/RHIContext.hxx"
#include "Engine/Core/RHI/RHIResource.hxx"

#include "Engine/Core/RHI/RHI.hxx"
#include "Engine/Core/RHI/RHIScene.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogRHI, Info);

class FGenericRHI
{
public:
    virtual ~FGenericRHI() = default;

    /// Initialize the RHI
    virtual void Init() = 0;

    /// Called after the RHI init
    virtual void PostInit() = 0;

    /// Called every frame from the main loop
    virtual void Tick(float fDeltaTime) = 0;

    /// Shutdown the RHI
    virtual void Shutdown() = 0;

    /// Return the name of the RHI
    virtual const char* GetName() const = 0;
    /// Return the type of the RHI
    virtual ERHIInterfaceType GetInterfaceType() const
    {
        return ERHIInterfaceType::Null;
    }

    /// @brief Defer the execution of the given function to the next frame
    /// @param InDeletionFunction The function to defer
    ///
    /// This function is used to defer the deletion of resources to the next frame, this is useful when the resource is
    /// in use and cannot be deleted immediately
    virtual void DeferedDeletion(std::function<void()>&& InDeletionFunction) = 0;
    virtual void FlushDeletionQueue() = 0;

    virtual void RegisterScene(WeakRef<RHIScene> Scene) = 0;

    virtual void WaitUntilIdle() = 0;

    // ---------------------- RHI Operations --------------------- //

    /// @brief Submit a list of command lists to the RHI
    virtual void RHISubmitCommandLists(FFRHICommandList* const CommandLists, std::uint32_t NumCommandLists) = 0;
    virtual FRHIContext* RHIGetCommandContext() = 0;
    virtual void RHIReleaseCommandContext(FRHIContext*) = 0;

    /// @copydoc RHI::CreateViewport
    virtual Ref<RRHIViewport> CreateViewport(Ref<RWindow> InWindowHandle, UVector2 InSize, bool bCreateDepthBuffer) = 0;
    /// @copydoc RHI::CreateTexture
    virtual Ref<RRHITexture> CreateTexture(const FRHITextureSpecification& InDesc) = 0;
    /// @copydoc RHI::CreateBuffer
    virtual Ref<RRHIBuffer> CreateBuffer(const FRHIBufferDesc& InDesc) = 0;
    /// @copydoc RHI::CreateShader
    virtual Ref<RRHIShader> CreateShader(const std::filesystem::path Path, bool bForceCompile) = 0;
    /// @copydoc RHI::CreateGraphicsPipeline
    virtual Ref<RRHIGraphicsPipeline> CreateGraphicsPipeline(const FRHIGraphicsPipelineSpecification& Config) = 0;
    /// @copydoc RHI::CreateMaterial
    virtual Ref<RRHIMaterial> CreateMaterial(const WeakRef<RRHIGraphicsPipeline>& Pipeline) = 0;
};

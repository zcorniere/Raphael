#pragma once

#include "Engine/Core/RHI/RHIContext.hxx"
#include "Engine/Core/RHI/RHIResource.hxx"

#include "Engine/Core/RHI/RHI.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogRHI, Info);

class GenericRHI
{
public:
    virtual ~GenericRHI() = default;

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
    virtual RHIInterfaceType GetInterfaceType() const
    {
        return RHIInterfaceType::Null;
    }

    /// @brief Defer the execution of the given function to the next frame
    /// @param InDeletionFunction The function to defer
    ///
    /// This function is used to defer the deletion of resources to the next frame, this is useful when the resource is
    /// in use and cannot be deleted immediately
    virtual void DeferedDeletion(std::function<void()>&& InDeletionFunction) = 0;
    virtual void FlushDeletionQueue() = 0;

    virtual void WaitUntilIdle() = 0;

    // ---------------------- RHI Operations --------------------- //

    /// @brief Submit a list of command lists to the RHI
    virtual void RHISubmitCommandLists(RHICommandList* const CommandLists, std::uint32_t NumCommandLists) = 0;
    virtual RHIContext* RHIGetCommandContext() = 0;
    virtual void RHIReleaseCommandContext(RHIContext*) = 0;

    /// @copydoc RHI::CreateViewport
    virtual Ref<RHIViewport> CreateViewport(Ref<Window> InWindowHandle, UVector2 InSize) = 0;
    /// @copydoc RHI::CreateTexture
    virtual Ref<RHITexture> CreateTexture(const RHITextureSpecification& InDesc) = 0;
    /// @copydoc RHI::CreateBuffer
    virtual Ref<RHIBuffer> CreateBuffer(const RHIBufferDesc& InDesc) = 0;
    /// @copydoc RHI::CreateShader
    virtual Ref<RHIShader> CreateShader(const std::filesystem::path Path, bool bForceCompile) = 0;
    /// @copydoc RHI::CreateGraphicsPipeline
    virtual Ref<RHIGraphicsPipeline> CreateGraphicsPipeline(const RHIGraphicsPipelineSpecification& Config) = 0;
};

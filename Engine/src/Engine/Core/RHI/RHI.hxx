#pragma once

#include "Engine/Core/ECS/Component/CameraComponent.hxx"
#include "Engine/Core/ECS/Component/MeshComponent.hxx"
#include "Engine/Core/RHI/RHIResource.hxx"
#include <Engine/Core/RHI/RHICommandList.hxx>

#include <future>

class RWindow;

DECLARE_LOGGER_CATEGORY(Core, LogRHI, Info);

enum class ERHIInterfaceType {
    Null,
    Vulkan,
};

class FFRHICommandList;

class FGenericRHI;

extern FGenericRHI* GDynamicRHI;

/// @brief This function create the RHI, and must be implemented individually by every RHI
extern "C" FGenericRHI* RHI_CreateRHI();

/// Wrapper function around the RHI function
namespace RHI
{

/// @brief Return the current RHI
/// @tparam TRHI The type of the RHI, default is GenericRHI
template <typename TRHI = FGenericRHI>
FORCEINLINE TRHI* Get()
{
    checkMsg(GDynamicRHI, "Attemped to fetch the RHI to early !");
    return static_cast<TRHI*>(GDynamicRHI);
}

/// @brief This function create the RHI, and perform early initialisation
void Create();

/// @brief Called every frame from the main loop
void Tick(float fDeltaTime);

/// @brief Delete the current RHI
void Destroy();

/// @brief Defer the execution of the given function to the next frame
/// @param InDeletionFunction The function to defer
///
/// This function is used to defer the deletion of resources to the next frame, this is useful when the resource is in
/// use and cannot be deleted immediately
void DeferedDeletion(std::function<void()>&& InDeletionFunction);
void FlushDeletionQueue();

/// -------------- RHI Operations --------------

/// @brief Mark the beginning of a new frame
void BeginFrame();
/// @brief Mark the end of the current frame
void EndFrame();

void RHIWaitUntilIdle();

/// Create a new RHI viewport - through the current RHI
Ref<RRHIViewport> CreateViewport(Ref<RWindow> InWindowHandle, UVector2 InSize);
/// Create a new RHI texture - through the current RHI
Ref<RRHITexture> CreateTexture(const FRHITextureSpecification& InDesc);
/// Create a new RHI buffer - through the current RHI
Ref<RRHIBuffer> CreateBuffer(const FRHIBufferDesc& InDesc);
/// Create a new RHI shader - through the current RHI
Ref<RRHIShader> CreateShader(const std::filesystem::path Path, bool bForceCompile);
/// Create a new RHI shader - through the current RHI asynchronously
std::future<Ref<RRHIShader>> CreateShaderAsync(const std::filesystem::path Path, bool bForceCompile);
/// Create a new RHI Pipeline - through the current RHI
Ref<RRHIGraphicsPipeline> CreateGraphicsPipeline(const FRHIGraphicsPipelineSpecification& Config);
/// Create a new RHI Material - through the current RHI
Ref<RRHIMaterial> CreateMaterial(const WeakRef<RRHIGraphicsPipeline>& Pipeline);

};    // namespace RHI

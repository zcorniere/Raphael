#pragma once

#include "Engine/Core/RHI/RHIResource.hxx"

class Window;

DECLARE_LOGGER_CATEGORY(Core, LogRHI, Info);

enum class RHIInterfaceType {
    Null,
    Vulkan,
};

class RHICommandList;

class GenericRHI;

extern GenericRHI* GDynamicRHI;

/// @brief This function create the RHI, and must be implemented individually by every RHI
extern "C" GenericRHI* RHI_CreateRHI();

/// Wrapper function around the RHI function
namespace RHI
{

/// @brief Return the current RHI
/// @tparam TRHI The type of the RHI, default is GenericRHI
template <typename TRHI = GenericRHI>
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

/// -------------- RHI Operations --------------

/// @brief Mark the beginning of a new frame
void BeginFrame();
/// @brief Mark the end of the current frame
void EndFrame();
void RHISubmitCommandLists(RHICommandList* const* CommandLists, std::uint32_t NumCommandLists);

/// @brief Indicate the RHI that we are starting drawing
void BeginRendering(const RHIRenderPassDescription& Renderpass, const RHIFramebufferDefinition& Framebuffer);
/// @brief Indicate the RHI that we are done rendering for now
void EndRendering();

/// Temp
void Draw(Ref<RHIGraphicsPipeline>& Pipeline);

/// Create a new RHI viewport - through the current RHI
Ref<RHIViewport> CreateViewport(Ref<Window> InWindowHandle, glm::uvec2 InSize);
/// Create a new RHI texture - through the current RHI
Ref<RHITexture> CreateTexture(const RHITextureSpecification& InDesc);
/// Create a new RHI buffer - through the current RHI
Ref<RHIBuffer> CreateBuffer(const RHIBufferDesc& InDesc);
/// Create a new RHI shader - through the current RHI
Ref<RHIShader> CreateShader(const std::filesystem::path Path, bool bForceCompile);
/// Create a new RHI Pipeline - through the current RHI
Ref<RHIGraphicsPipeline> CreateGraphicsPipeline(const RHIGraphicsPipelineSpecification& Config);

};    // namespace RHI

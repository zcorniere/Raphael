#pragma once

#include "Engine/Core/RHI/RHIResource.hxx"

class Window;

DECLARE_LOGGER_CATEGORY(Core, LogGenericRHI, Info);

enum class RHIInterfaceType {
    Null,
    Vulkan,
};

class RHICommandQueue;

class GenericRHI;
extern std::unique_ptr<GenericRHI> GDynamicRHI;

/// Wrapper static arround the RHI function
class RHI
{
public:
    /// @brief Return the current RHI
    /// @tparam TRHI The type of the RHI, default is GenericRHI
    template <typename TRHI = GenericRHI>
    FORCEINLINE static TRHI* Get()
    {
        checkMsg(GDynamicRHI, "Attemped to fetch the RHI to early !");
        return static_cast<TRHI*>(GDynamicRHI.get());
    }

    /// @brief This function create RHI-agnostic object, like the command queue
    static void Init();
    /// @brief This function create the RHI, and must be implemented individualy by every RHI
    static GenericRHI* CreateRHI();

    /// @brief Delete the current RHI
    static void DeleteRHI();

    /// @brief Return the command queue of the RHI
    static RHICommandQueue* GetRHICommandQueue();

    /// -------------- RHI Operations --------------

    /// @brief Mark the begining of a new frame
    static void BeginFrame();
    /// @brief Mark the end of the current frame
    static void EndFrame();
    /// @brief Indicate the RHI that we are moving to a new frame
    static void NextFrame();
    /// @brief Indicate the RHI that we are starting drawing
    static void BeginRenderPass(const RHIRenderPassDescription& Renderpass,
                                const RHIFramebufferDefinition& Framebuffer);
    /// @brief Indicate the RHI that we are done rendering for now
    static void EndRenderPass();

    /// Temp
    static void Draw(Ref<RHIGraphicsPipeline>& Pipeline);

    /// Create a new RHI viewport - through the current RHI
    static Ref<RHIViewport> CreateViewport(Ref<Window> InWindowHandle, glm::uvec2 InSize);
    /// Create a new RHI texture - through the current RHI
    static Ref<RHITexture> CreateTexture(const RHITextureSpecification& InDesc);
    /// Create a new RHI buffer - through the current RHI
    static Ref<RHIBuffer> CreateBuffer(const RHIBufferDesc& InDesc);
    /// Create a new RHI shader - through the current RHI
    static Ref<RHIShader> CreateShader(const std::filesystem::path Path, bool bForceCompile);
    /// Create a new RHI Pipeline - through the current RHI
    static Ref<RHIGraphicsPipeline> CreateGraphicsPipeline(const RHIGraphicsPipelineSpecification& Config);
};

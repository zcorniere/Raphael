#pragma once

#include "Engine/Core/RHI/RHICommandQueue.hxx"
#include "Engine/Core/RHI/RHIResource.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogGenericRHI, Info);

enum class RHIInterfaceType {
    Null,
    Vulkan,
};

class GenericRHI;
extern Ref<GenericRHI> GDynamicRHI;

/// Wrapper static arround the RHI function
class RHI
{
public:
    /// @brief Return the current RHI
    /// @tparam TRHI The type of the RHI, default is GenericRHI
    template <typename TRHI = GenericRHI>
    static Ref<TRHI> Get()
    {
        check(GDynamicRHI);
        return GDynamicRHI.As<TRHI>();
    }

    /// @brief This function create RHI-agnostic object, like the command queue
    static void Init();
    /// @brief This function create the RHI, and must be implemented individualy by every RHI
    static Ref<GenericRHI> CreateRHI();

    /// @brief Delete the current RHI
    static void DeleteRHI();

    /// @brief Subtmit a function to the Renderqueue, to be executed later
    template <typename TFunction>
    static void Submit(TFunction&& func)
    {
        RHI::GetRHICommandQueue()->EnqueueCommand(std::forward<TFunction>(func));
    }

    /// @brief Return the command queue of the RHI
    static Ref<RHICommandQueue>& GetRHICommandQueue();

    /// -------------- RHI Operations --------------

    /// @brief Mark the begining of a new frame
    static void BeginFrame();
    /// @brief Mark the end of the current frame
    static void EndFrame();
    /// @brief Indicate the RHI that we are moving to a new frame
    static void NextFrame();
    /// @brief Indicate the RHI that we are starting drawing
    static void BeginRenderPass(const RHIRenderPassDescription& Description);
    /// @brief Indicate the RHI that we are done rendering for now
    static void EndRenderPass();

    /// Create a new RHI viewport - through the current RHI
    static Ref<RHIViewport> CreateViewport(void* InWindowHandle, glm::uvec2 InSize);
    /// Create a new RHI texture - through the current RHI
    static Ref<RHITexture> CreateTexture(const RHITextureCreateDesc& InDesc);
    /// Create a new RHI buffer - through the current RHI
    static Ref<RHIBuffer> CreateBuffer(const uint32 InSize, const EBufferUsageFlags InUsage, const uint32 InStride,
                                       Ref<ResourceArray>& InitialData);
    /// Create a new RHI shader - through the current RHI
    static Ref<RHIShader> CreateShader(const std::filesystem::path Path, bool bForceCompile);
    /// Create a new RHI Pipeline - through the current RHI
    static Ref<RHIGraphicsPipeline> CreateGraphicsPipeline(const RHIGraphicsPipelineInitializer& Config);
};

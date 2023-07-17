#pragma once

DECLARE_LOGGER_CATEGORY(Core, LogVulkanRHI, Info);

#include "Engine/Core/RHI/GenericRHI.hxx"
#include "VulkanRHI/VulkanShaderCompiler.hxx"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

// Raphael classes
class RHIResource;

namespace VulkanRHI
{

class RenderPassManager;
class VulkanRenderPass;
class VulkanDevice;

/// @brief Vulkan RHI implementation for Raphael
class VulkanDynamicRHI : public GenericRHI
{
public:
    // ---------------------- RHI Operations --------------------- //
    virtual void BeginFrame() override;
    virtual void EndFrame() override;
    virtual void NextFrame() override;
    virtual void BeginRenderPass(const RHIRenderPassDescription& Description) override;
    virtual void EndRenderPass() override;

    virtual Ref<RHIViewport> CreateViewport(void* InWindowHandle, glm::uvec2 InSize) override;
    virtual Ref<RHITexture> CreateTexture(const RHITextureCreateDesc& InDesc) override;
    virtual Ref<RHIBuffer> CreateBuffer(const uint32 InSize, const EBufferUsageFlags InUsage, const uint32 InStride,
                                        Ref<ResourceArray>& InitialData) override;
    virtual Ref<RHIShader> CreateShader(const std::filesystem::path Path, bool bForceCompile) override;
    virtual Ref<RHIGraphicsPipeline> CreateGraphicsPipeline(const RHIGraphicsPipelineInitializer& Config) override;

public:
    VulkanDynamicRHI();
    virtual ~VulkanDynamicRHI();

    /// Return the Vulkan Device of the RHI
    VkDevice RHIGetVkDevice() const;
    /// Return the Vulkan Physical Device of the RHI
    VkPhysicalDevice RHIGetVkPhysicalDevice() const;

    virtual void Init() final override;
    virtual void PostInit() final override;
    virtual void Shutdown() final override;

    RHIInterfaceType GetInterfaceType() const final
    {
        return RHIInterfaceType::Vulkan;
    }

    inline VkInstance GetInstance() const
    {
        return m_Instance;
    }

    Ref<VulkanDevice>& GetDevice();

private:
    void CreateInstance();
    void SelectDevice();

#if VULKAN_DEBUGGING_ENABLED
    VkDebugUtilsMessengerEXT Messenger = VK_NULL_HANDLE;
    bool bValidationLayersAreMissing = false;

    Array<const char*> GetSupportedInstanceLayers();
    void SetupDebugLayerCallback();
    void RemoveDebugLayerCallback();
#endif

private:
    friend class VulkanViewport;
    void RT_SetDrawingViewport(WeakRef<VulkanViewport> Viewport);

private:
    VkInstance m_Instance;
    Ref<VulkanDevice> Device;

    WeakRef<VulkanViewport> DrawingViewport;

    RenderPassManager* RPassManager = nullptr;
    WeakRef<VulkanRenderPass> CurrentRenderPass;

    VulkanShaderCompiler ShaderCompiler;
};

}    // namespace VulkanRHI

FORCEINLINE Ref<VulkanRHI::VulkanDynamicRHI> GetVulkanDynamicRHI()
{
    checkMsg(GDynamicRHI, "Tried to fetch RHI too early");
    check(GDynamicRHI->GetInterfaceType() == RHIInterfaceType::Vulkan);
    return RHI::Get<VulkanRHI::VulkanDynamicRHI>();
}

#pragma once

DECLARE_LOGGER_CATEGORY(Core, LogVulkanRHI, Trace);

#include "Engine/Core/Memory/SmartPointers.hxx"
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
class VulkanViewport;

extern VkAllocationCallbacks GAllocationCallbacks;
static FORCEINLINE const VkAllocationCallbacks* GetMemoryAllocator()
{
    return &GAllocationCallbacks;
}

/// @brief Vulkan RHI implementation for Raphael
class VulkanDynamicRHI : public GenericRHI
{
public:
    // ---------------------- RHI Operations --------------------- //
    virtual void BeginFrame() override;
    virtual void Tick(float fDeltaTime) override;
    virtual void EndFrame() override;
    virtual void BeginRenderPass(const RHIRenderPassDescription& Renderpass,
                                 const RHIFramebufferDefinition& Framebuffer) override;
    virtual void EndRenderPass() override;

    virtual void Draw(Ref<RHIGraphicsPipeline>& Pipeline) override;

    virtual Ref<RHIViewport> CreateViewport(Ref<Window> InWindowHandle, glm::uvec2 InSize) override;
    virtual Ref<RHITexture> CreateTexture(const RHITextureSpecification& InDesc) override;
    virtual Ref<RHIBuffer> CreateBuffer(const RHIBufferDesc& InDesc) override;
    virtual Ref<RHIShader> CreateShader(const std::filesystem::path Path, bool bForceCompile) override;
    virtual Ref<RHIGraphicsPipeline> CreateGraphicsPipeline(const RHIGraphicsPipelineSpecification& Config) override;

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

    virtual const char* GetName() const final override
    {
        return "Vulkan";
    }
    RHIInterfaceType GetInterfaceType() const final
    {
        return RHIInterfaceType::Vulkan;
    }

    inline VkInstance GetInstance() const
    {
        return m_Instance;
    }

    VulkanDevice* GetDevice()
    {
        return Device.Get();
    }

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
    VkInstance m_Instance = VK_NULL_HANDLE;
    UniquePtr<VulkanDevice> Device = nullptr;
    UniquePtr<VulkanShaderCompiler> ShaderCompiler = nullptr;
    UniquePtr<RenderPassManager> RPassManager = nullptr;

    // Used during runtime //
    WeakRef<VulkanViewport> DrawingViewport = nullptr;
    WeakRef<VulkanRenderPass> CurrentRenderPass = nullptr;
};

}    // namespace VulkanRHI

FORCEINLINE VulkanRHI::VulkanDynamicRHI* GetVulkanDynamicRHI()
{
    checkMsg(GDynamicRHI, "Tried to fetch RHI too early");
    check(GDynamicRHI->GetInterfaceType() == RHIInterfaceType::Vulkan);
    return RHI::Get<VulkanRHI::VulkanDynamicRHI>();
}

#if VULKAN_CUSTOM_CPU_ALLOCATOR == 1
    #define VULKAN_CPU_ALLOCATOR ::VulkanRHI::GetMemoryAllocator()
#else
    #define VULKAN_CPU_ALLOCATOR nullptr
#endif

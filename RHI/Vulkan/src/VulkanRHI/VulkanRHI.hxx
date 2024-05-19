#pragma once

DECLARE_LOGGER_CATEGORY(Core, LogVulkanRHI, Trace);

#include "Engine/Core/RHI/GenericRHI.hxx"

#include "VulkanRHI/VulkanRHI_Debug.hxx"
#include "VulkanRHI/VulkanShaderCompiler.hxx"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

// Raphael classes
class RHIResource;

namespace VulkanRHI
{

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
    virtual void Tick(float fDeltaTime) override;

    // ---------------------- RHI Operations --------------------- //
    virtual void RHISubmitCommandLists(RHICommandList* const CommandLists, std::uint32_t NumCommandLists) override;
    virtual RHIContext* RHIGetCommandContext() override;
    virtual void RHIReleaseCommandContext(RHIContext* Context) override;

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

    virtual void WaitUntilIdle() final override;

    virtual const char* GetName() const final override
    {
        return "Vulkan";
    }
    RHIInterfaceType GetInterfaceType() const final override
    {
        return RHIInterfaceType::Vulkan;
    }

    inline VkInstance GetInstance() const
    {
        return m_Instance;
    }

    VulkanDevice* GetDevice()
    {
        return Device.get();
    }

private:
    static VkInstance CreateInstance(const Array<const char*>& ValidationLayers);
    static VulkanDevice* SelectDevice(VkInstance Instance);

private:
    friend class VulkanCommandContext;

private:
#if VULKAN_DEBUGGING_ENABLED
    VulkanRHI_Debug DebugLayer;
#endif
    VkInstance m_Instance = VK_NULL_HANDLE;
    std::unique_ptr<VulkanDevice> Device;

    std::unique_ptr<VulkanShaderCompiler> ShaderCompiler;

    // Used during runtime //
    VulkanViewport* DrawingViewport = nullptr;
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

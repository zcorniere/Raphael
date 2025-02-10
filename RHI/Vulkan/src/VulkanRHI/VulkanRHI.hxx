#pragma once

DECLARE_LOGGER_CATEGORY(Core, LogVulkanRHI, Trace);

#include "Engine/Core/RHI/GenericRHI.hxx"

#include "VulkanRHI/VulkanRHI_Debug.hxx"
#include "VulkanRHI/VulkanShaderCompiler.hxx"

// Raphael classes
class RRHIResource;

namespace VulkanRHI
{

class FVulkanDevice;
class RVulkanViewport;
class FVulkanCommandContext;

extern VkAllocationCallbacks GAllocationCallbacks;
static FORCEINLINE const VkAllocationCallbacks* GetMemoryAllocator()
{
    return &GAllocationCallbacks;
}

/// @brief Vulkan RHI implementation for Raphael
class FVulkanDynamicRHI : public FGenericRHI
{
public:
    // FGenericRHI implementation
    virtual void Init() final override;
    virtual void PostInit() final override;
    virtual void Tick(float fDeltaTime) override;
    virtual void Shutdown() final override;

    virtual const char* GetName() const final override
    {
        return "Vulkan";
    }
    virtual ERHIInterfaceType GetInterfaceType() const final override
    {
        return ERHIInterfaceType::Vulkan;
    }

    virtual void DeferedDeletion(std::function<void()>&& InDeletionFunction) final override;
    virtual void FlushDeletionQueue() final override;

    virtual void RegisterScene(WeakRef<RHIScene> Scene) final override;

    virtual void WaitUntilIdle() final override;

    // ---------------------- RHI Operations --------------------- //
    virtual void RHISubmitCommandLists(FFRHICommandList* const CommandLists, std::uint32_t NumCommandLists) override;
    virtual FRHIContext* RHIGetCommandContext() override;
    virtual void RHIReleaseCommandContext(FRHIContext* Context) override;

    virtual Ref<RRHIViewport> CreateViewport(Ref<RWindow> InWindowHandle, UVector2 InSize) override;
    virtual Ref<RRHITexture> CreateTexture(const FRHITextureSpecification& InDesc) override;
    virtual Ref<RRHIBuffer> CreateBuffer(const FRHIBufferDesc& InDesc) override;
    virtual Ref<RRHIShader> CreateShader(const std::filesystem::path Path, bool bForceCompile) override;
    virtual Ref<RRHIGraphicsPipeline> CreateGraphicsPipeline(const FRHIGraphicsPipelineSpecification& Config) override;

public:
    FVulkanDynamicRHI();
    virtual ~FVulkanDynamicRHI();

    /// Return the Vulkan Device of the RHI
    VkDevice RHIGetVkDevice() const;
    /// Return the Vulkan Physical Device of the RHI
    VkPhysicalDevice RHIGetVkPhysicalDevice() const;

    inline VkInstance GetInstance() const
    {
        return m_Instance;
    }

    FVulkanDevice* GetDevice()
    {
        return Device.get();
    }

private:
    static VkInstance CreateInstance(const TArray<const char*>& ValidationLayers);
    static FVulkanDevice* SelectDevice(VkInstance Instance);

private:
    friend class FVulkanCommandContext;

private:
#if VULKAN_DEBUGGING_ENABLED
    VulkanRHI_Debug DebugLayer;
#endif
    VkInstance m_Instance = VK_NULL_HANDLE;
    std::unique_ptr<FVulkanDevice> Device;

    std::unique_ptr<FVulkanShaderCompiler> ShaderCompiler;

    // Used during runtime //
    TArray<FVulkanCommandContext*> CommandContexts;
    TArray<FVulkanCommandContext*> AvailableCommandContexts;
    RVulkanViewport* DrawingViewport = nullptr;

    TArray<WeakRef<RHIScene>> ScenesContainers;

    TArray<std::function<void()>> DeletionQueue;
};

}    // namespace VulkanRHI

FORCEINLINE VulkanRHI::FVulkanDynamicRHI* GetVulkanDynamicRHI()
{
    checkMsg(GDynamicRHI, "Tried to fetch RHI too early");
    check(GDynamicRHI->GetInterfaceType() == ERHIInterfaceType::Vulkan);
    return RHI::Get<VulkanRHI::FVulkanDynamicRHI>();
}

#if VULKAN_CUSTOM_CPU_ALLOCATOR == 1
    #define VULKAN_CPU_ALLOCATOR ::VulkanRHI::GetMemoryAllocator()
#else
    #define VULKAN_CPU_ALLOCATOR nullptr
#endif

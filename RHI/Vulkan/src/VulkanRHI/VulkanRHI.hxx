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

class VulkanDevice;

class VulkanDynamicRHI : public GenericRHI
{
public:
    // ---------------------- RHI Operations --------------------- //
    virtual void BeginFrame() override;
    virtual void EndFrame() override;
    virtual void NextFrame() override;

    virtual Ref<RHIViewport> CreateViewport(void* InWindowHandle, glm::uvec2 InSize) override;
    virtual Ref<RHITexture> CreateTexture(const RHITextureCreateDesc InDesc) override;
    virtual Ref<RHIShader> CreateShader(const std::filesystem::path Path, bool bForceCompile) override;

public:
    VulkanDynamicRHI();
    virtual ~VulkanDynamicRHI();

    virtual VkInstance RHIGetVkInstance() const;
    virtual VkDevice RHIGetVkDevice() const;
    virtual VkPhysicalDevice RHIGetVkPhysicalDevice() const;

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

    Ref<VulkanDevice> GetDevice();

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

    Array<Ref<VulkanViewport>> Viewports;
    WeakRef<VulkanViewport> DrawingViewport;

    VulkanShaderCompiler ShaderCompiler;
};

}    // namespace VulkanRHI

FORCEINLINE Ref<VulkanRHI::VulkanDynamicRHI> GetVulkanDynamicRHI()
{
    checkMsg(GDynamicRHI, "Tried to fetch RHI too early");
    check(GDynamicRHI->GetInterfaceType() == RHIInterfaceType::Vulkan);
    return RHI::Get<VulkanRHI::VulkanDynamicRHI>();
}

#pragma once

DECLARE_LOGGER_CATEGORY(Core, LogVulkanRHI, Info);

#include "Engine/Core/RHI/GenericRHI.hxx"
#include "RHI/Vulkan/VulkanShaderCompiler.hxx"

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
    static void BeginFrame();
    static void EndFrame();
    static void NextFrame();

    template <RHIResourceType Type, typename... Args>
    static Ref<RHIResource> Create(Args... args);

public:
    VulkanDynamicRHI();
    virtual ~VulkanDynamicRHI();

    virtual VkInstance RHIGetVkInstance() const;
    virtual VkDevice RHIGetVkDevice() const;
    virtual VkPhysicalDevice RHIGetVkPhysicalDevice() const;

    virtual void Init() final override;
    virtual void PostInit() final override;
    virtual void Shutdown() final override;

    RHIInterfaceType GetInterfaceType() const final { return RHIInterfaceType::Vulkan; }

    inline VkInstance GetInstance() const { return m_Instance; }

    Ref<VulkanDevice> GetDevice();

protected:
    void CreateInstance();
    void SelectDevice();

#if VULKAN_DEBUGGING_ENABLED
    VkDebugUtilsMessengerEXT Messenger = VK_NULL_HANDLE;

    void SetupDebugLayerCallback();
    void RemoveDebugLayerCallback();
#endif

protected:
    VkInstance m_Instance;
    Ref<VulkanDevice> Device;

    VulkanShaderCompiler ShaderCompiler;
};

}    // namespace VulkanRHI

FORCEINLINE Ref<VulkanRHI::VulkanDynamicRHI> GetVulkanDynamicRHI()
{
    checkMsg(GDynamicRHI, "Tried to fetch RHI too early");
    check(GDynamicRHI->GetInterfaceType() == RHIInterfaceType::Vulkan);
    static_assert(std::is_same_v<RHI, VulkanRHI::VulkanDynamicRHI>);
    return RHI::Get<VulkanRHI::VulkanDynamicRHI>();
}

using RHI = VulkanRHI::VulkanDynamicRHI;

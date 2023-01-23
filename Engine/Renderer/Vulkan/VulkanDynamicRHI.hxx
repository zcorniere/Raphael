#pragma once

#include "Engine/Renderer/Vulkan/IVulkanDynamicRHI.hxx"

namespace Raphael::RHI
{

class VulkanDevice;

class VulkanDynamicRHI : public IVulkanDynamicRHI
{
public:
    VulkanDynamicRHI();
    ~VulkanDynamicRHI()
    {
    }

    virtual VkInstance RHIGetVkInstance() const final override;
    virtual VkDevice RHIGetVkDevice() const final override;
    virtual VkPhysicalDevice RHIGetVkPhysicalDevice() const final override;

    virtual void Init() final override;
    virtual void PostInit() final override;
    virtual void Shutdown() final override;
    virtual const char *GetName() final override
    {
        return "Vulkan";
    }

    inline VkInstance GetInstance() const
    {
        return m_Instance;
    }

protected:
    void CreateInstance();
    void SelectDevice();
    void InitGPU(VulkanDevice *Device);
    void InitDevice(VulkanDevice *Device);

#if VULKAN_DEBUGGING_ENABLED
    VkDebugUtilsMessengerEXT Messenger = VK_NULL_HANDLE;

    void SetupDebugLayerCallback();
    void RemoveDebugLayerCallback();
#endif

protected:
    VkInstance m_Instance;
    VulkanDevice *Device;
};

}    // namespace Raphael::RHI

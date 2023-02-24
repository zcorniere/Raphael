#pragma once

#include "Engine/Renderer/Vulkan/IVulkanDynamicRHI.hxx"

namespace VulkanRHI
{

class VulkanDevice;

class VulkanDynamicRHI : public IVulkanDynamicRHI
{
public:
    VulkanDynamicRHI();
    ~VulkanDynamicRHI();

    virtual VkInstance RHIGetVkInstance() const final override;
    virtual VkDevice RHIGetVkDevice() const final override;
    virtual VkPhysicalDevice RHIGetVkPhysicalDevice() const final override;

    virtual void Init() final override;
    virtual void PostInit() final override;
    virtual void Shutdown() final override;

    inline VkInstance GetInstance() const
    {
        return m_Instance;
    }

    Ref<VulkanDevice> GetDevice();

    void InitInstance();

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

    std::vector<Ref<VulkanDevice>> Devices;
};

}    // namespace VulkanRHI

#pragma once

#include "Engine/Renderer/Renderer.hxx"
#include "Engine/Renderer/RendererContext.hxx"
#include "Engine/Renderer/Vulkan/VulkanDevice.hxx"
#include "Engine/Renderer/Vulkan/VulkanPhysicalDevice.hxx"

namespace Raphael
{

class VulkanContext : public RendererContext
{
public:
    VulkanContext();
    virtual ~VulkanContext();

    virtual void Init() override;

    Ref<VulkanDevice> GetDevice()
    {
        return m_Device;
    }
    static VkInstance GetInstance()
    {
        return s_VulkanInstance;
    }
    static Ref<VulkanContext> Get()
    {
        return Ref<VulkanContext>(Renderer::GetContext());
    }

    static Ref<VulkanDevice> GetCurrentDevice()
    {
        return Get()->GetDevice();
    }

private:
    Ref<VulkanPhysicalDevice> m_PhysicalDevice;
    Ref<VulkanDevice> m_Device;

    inline static VkInstance s_VulkanInstance;

    VkDebugUtilsMessengerEXT m_DebugUtilsMessenger = nullptr;
};

}    // namespace Raphael

#pragma once

#include "Engine/Core/RHI/Resources/RHIViewport.hxx"

#include "RHI/Vulkan/Resources/VulkanTexture.hxx"

namespace VulkanRHI
{

class VulkanDevice;
class Semaphore;
class VulkanSwapChain;

class VulkanViewport : public RHIViewport
{
public:
    VulkanViewport(Ref<VulkanDevice> InDevice, void *InWindowHandle, glm::uvec2 InSize);
    ~VulkanViewport();

    void SetName(std::string_view InName) override;

private:
    void CreateSwapchain();

private:
    Ref<VulkanDevice> Device;
    Ref<VulkanSwapChain> SwapChain;

    std::vector<VkImage> BackBufferImages;
    std::vector<VulkanTextureView> TexturesViews;
    std::vector<Ref<Semaphore>> RenderingDoneSemaphores;

    void *WindowHandle;
    glm::uvec2 Size;
};

}    // namespace VulkanRHI

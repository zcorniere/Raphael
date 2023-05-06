#pragma once

#include "Engine/Core/RHI/Resources/RHIViewport.hxx"

#include "VulkanRHI/Resources/VulkanTexture.hxx"

namespace VulkanRHI
{

class VulkanDevice;
class Semaphore;
class VulkanSwapChain;
class VulkanQueue;
class VulkanCmdBuffer;
struct VulkanSwapChainRecreateInfo;

class VulkanViewport : public RHIViewport
{
public:
    VulkanViewport(Ref<VulkanDevice> InDevice, void *InWindowHandle, glm::uvec2 InSize);
    ~VulkanViewport();

    void SetName(std::string_view InName) override;
    bool Present(Ref<VulkanCmdBuffer> &CmdBuffer, Ref<VulkanQueue> &Queue, Ref<VulkanQueue> &PresentQueue);
    void RecreateSwapchain(void *NewNativeWindow);

private:
    void CreateSwapchain(VulkanSwapChainRecreateInfo *RecreateInfo);
    void DeleteSwapchain(VulkanSwapChainRecreateInfo *RecreateInfo);
    bool TryAcquireImageIndex();

private:
    Ref<VulkanDevice> Device;
    Ref<VulkanSwapChain> SwapChain;

    std::vector<VkImage> BackBufferImages;
    std::vector<VulkanTextureView> TexturesViews;
    std::vector<Ref<Semaphore>> RenderingDoneSemaphores;

    void *WindowHandle;
    glm::uvec2 Size;

    int32 AcquiredImageIndex;
    Ref<Semaphore> AcquiredSemaphore;
};

}    // namespace VulkanRHI

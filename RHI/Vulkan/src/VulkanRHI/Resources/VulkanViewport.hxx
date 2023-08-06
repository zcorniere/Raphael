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
    VulkanViewport(VulkanDevice* InDevice, void* InWindowHandle, glm::uvec2 InSize);
    ~VulkanViewport();

    virtual glm::uvec2 GetSize() const override
    {
        return Size;
    }

    void SetName(std::string_view InName) override;
    bool Present(VulkanCmdBuffer* CmdBuffer, VulkanQueue* Queue, VulkanQueue* PresentQueue);
    void RecreateSwapchain(void* NewNativeWindow);

private:
    // Override from RHIViewport
    virtual void RT_BeginDrawViewport() override;
    virtual void RT_EndDrawViewport() override;
    virtual void RT_ResizeViewport(uint32 Width, uint32 Height) override;

private:
    void CreateSwapchain(VulkanSwapChainRecreateInfo* RecreateInfo);
    void DeleteSwapchain(VulkanSwapChainRecreateInfo* RecreateInfo);
    bool TryAcquireImageIndex();

    bool TryPresenting(VulkanQueue* PresentQueue);

private:
    VulkanDevice* Device;
    Ref<VulkanSwapChain> SwapChain;

    Array<VkImage> BackBufferImages;
    Array<VulkanTextureView> TexturesViews;
    Array<Ref<Semaphore>> RenderingDoneSemaphores;

    void* WindowHandle;
    glm::uvec2 Size;

    int32 AcquiredImageIndex;
    Ref<Semaphore> AcquiredSemaphore;

    friend class VulkanDynamicRHI;
};

}    // namespace VulkanRHI

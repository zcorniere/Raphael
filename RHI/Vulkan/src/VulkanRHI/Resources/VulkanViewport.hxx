#pragma once

#include "Engine/Core/RHI/Resources/RHIViewport.hxx"

#include "VulkanRHI/Resources/VulkanTexture.hxx"

class Window;

namespace VulkanRHI
{

class VulkanDevice;
class Semaphore;
class VulkanSwapChain;
class VulkanQueue;
class VulkanCmdBuffer;
struct VulkanSwapChainRecreateInfo;

class VulkanCommandContext;

class VulkanViewport : public RHIViewport, public IDeviceChild
{
public:
    VulkanViewport(VulkanDevice* InDevice, Ref<Window> InWindowHandle, glm::uvec2 InSize);
    ~VulkanViewport();

    virtual glm::uvec2 GetSize() const override
    {
        return RenderingBackbuffer->GetDescription().Extent;
    }
    virtual void ResizeViewport(uint32 Width, uint32 Height) override;

    void SetName(std::string_view InName) override;
    bool Present(VulkanCommandContext* Context, VulkanCmdBuffer* CmdBuffer, VulkanQueue* Queue,
                 VulkanQueue* PresentQueue);
    void RecreateSwapchain(Ref<Window> NewNativeWindow);

    virtual Ref<RHITexture> GetBackbuffer() const override
    {
        check(RenderingBackbuffer);
        return RenderingBackbuffer.As<RHITexture>();
    }

private:
    void CreateSwapchain(VulkanSwapChainRecreateInfo* RecreateInfo);
    void DeleteSwapchain(VulkanSwapChainRecreateInfo* RecreateInfo);
    bool TryAcquireImageIndex();

    bool TryPresenting(VulkanQueue* PresentQueue);

private:
    Ref<VulkanSwapChain> SwapChain;

    Array<VkImage> BackBufferImages;
    Array<VulkanTextureView> TexturesViews;
    Array<Ref<Semaphore>> RenderingDoneSemaphores;

    Ref<VulkanTexture> RenderingBackbuffer;
    Ref<Window> WindowHandle;
    glm::uvec2 Size;

    int32 AcquiredImageIndex;
    Ref<Semaphore> AcquiredSemaphore;

    friend class VulkanDynamicRHI;
};

}    // namespace VulkanRHI

#pragma once

#include "Engine/Core/RHI/Resources/RHIViewport.hxx"

#include "VulkanRHI/Resources/VulkanTexture.hxx"

class RWindow;

namespace VulkanRHI
{

class FVulkanDevice;
class RSemaphore;
class RVulkanSwapChain;
class FVulkanQueue;
class FVulkanCmdBuffer;
struct VulkanSwapChainRecreateInfo;

class FVulkanCommandContext;

class VulkanViewport : public RRHIViewport, public IDeviceChild
{
    RTTI_DECLARE_TYPEINFO(VulkanViewport, RRHIViewport);

public:
    VulkanViewport(FVulkanDevice* InDevice, Ref<RWindow> InWindowHandle, UVector2 InSize);
    ~VulkanViewport();

    virtual UVector2 GetSize() const override
    {
        return RenderingBackbuffer->GetDescription().Extent;
    }
    virtual void ResizeViewport(uint32 Width, uint32 Height) override;

    void SetName(std::string_view InName) override;
    bool Present(FVulkanCommandContext* Context, FVulkanCmdBuffer* CmdBuffer, FVulkanQueue* Queue,
                 FVulkanQueue* PresentQueue);
    void RecreateSwapchain(Ref<RWindow> NewNativeWindow);

    virtual Ref<RRHITexture> GetBackbuffer() const override
    {
        check(RenderingBackbuffer);
        return RenderingBackbuffer.As<RRHITexture>();
    }

private:
    void CreateSwapchain(VulkanSwapChainRecreateInfo* RecreateInfo);
    void DeleteSwapchain(VulkanSwapChainRecreateInfo* RecreateInfo);
    bool TryAcquireImageIndex();

    bool TryPresenting(FVulkanQueue* PresentQueue);

private:
    Ref<RVulkanSwapChain> SwapChain;

    TArray<VkImage> BackBufferImages;
    TArray<VulkanTextureView> TexturesViews;
    TArray<Ref<RSemaphore>> RenderingDoneSemaphores;

    Ref<VulkanTexture> RenderingBackbuffer;
    Ref<RWindow> WindowHandle;
    UVector2 Size;

    int32 AcquiredImageIndex;
    Ref<RSemaphore> AcquiredSemaphore;

    friend class FVulkanDynamicRHI;
};

}    // namespace VulkanRHI

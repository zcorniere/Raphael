#pragma once

#include "Engine/Core/RHI/Resources/RHIViewport.hxx"

#include "VulkanRHI/Resources/VulkanTexture.hxx"

#include "Engine/Core/Window.hxx"
#include "VulkanRHI/VulkanSwapChain.hxx"

namespace VulkanRHI
{

class FVulkanDevice;
class RSemaphore;
class FVulkanQueue;
class FVulkanCmdBuffer;
struct VulkanSwapChainRecreateInfo;

class FVulkanCommandContext;

class RVulkanViewport : public RRHIViewport, public IDeviceChild
{
    RTTI_DECLARE_TYPEINFO(RVulkanViewport, RRHIViewport);

public:
    RVulkanViewport(FVulkanDevice* InDevice, Ref<RWindow> InWindowHandle, UVector2 InSize, bool bCreateDepthBuffer);
    virtual ~RVulkanViewport();

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

    virtual Ref<RRHITexture> GetDepthBuffer() const override
    {
        return DepthBuffer.As<RRHITexture>();
    }

    virtual Ref<RSlate> GetSlateInstance(bool bCreate = false) override;

private:
    void CreateSwapchain(VulkanSwapChainRecreateInfo* RecreateInfo);
    void DeleteSwapchain(VulkanSwapChainRecreateInfo* RecreateInfo);
    bool TryAcquireImageIndex();

    bool TryPresenting(FVulkanQueue* PresentQueue);

private:
    Ref<RVulkanSwapChain> SwapChain = nullptr;

    TArray<VkImage> BackBufferImages = {};
    TArray<VulkanTextureView> TexturesViews = {};
    TArray<Ref<RSemaphore>> RenderingDoneSemaphores = {};

    Ref<RVulkanTexture> RenderingBackbuffer = nullptr;
    Ref<RVulkanTexture> DepthBuffer = nullptr;
    bool bCreateDepthBuffer = false;

    Ref<RWindow> WindowHandle = nullptr;
    UVector2 Size = {0, 0};

    int32 AcquiredImageIndex = -1;
    Ref<RSemaphore> AcquiredSemaphore = nullptr;

    Ref<RSlate> SlateInstance = nullptr;

    friend class FVulkanDynamicRHI;
};

}    // namespace VulkanRHI

#pragma once

class RWindow;

namespace VulkanRHI
{

class FVulkanTexture;
class FVulkanDevice;
class RSemaphore;
class RFence;
class FVulkanQueue;

struct VulkanSwapChainRecreateInfo {
    VkSwapchainKHR SwapChain = VK_NULL_HANDLE;
    VkSurfaceKHR Surface = VK_NULL_HANDLE;
};

class RVulkanSwapChain : public RObject, public IDeviceChild
{
public:
    enum class EStatus {
        Healty = 0,
        OutOfDate = -1,
        SurfaceLost = -2,
    };

private:
    /// @brief Helper class to gather all information required for swapchain creation
    class FSupportDetails
    {
    public:
        /// Gather swapchain support information
        static FSupportDetails QuerySwapChainSupport(const FVulkanDevice* Device, const VkSurfaceKHR& Surface);

    public:
        /// Choose a fitting format
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat() const noexcept;
        /// Choose a presentation mode
        VkPresentModeKHR ChooseSwapPresentMode(bool LockToVSync) const noexcept;
        /// Check if the size if supported
        VkExtent2D ChooseSwapExtent(const UVector2& InSize) const noexcept;

    public:
        /// surface capability
        VkSurfaceCapabilitiesKHR Capabilities;
        /// List of supported format
        TArray<VkSurfaceFormatKHR> Formats;
        /// List of supported presentation modes
        TArray<VkPresentModeKHR> PresentModes;
    };

public:
    RVulkanSwapChain(VkInstance InInstance, FVulkanDevice* InDevice, const UVector2& InSize, RWindow* WindowHandle,
                     uint32 InOutDesiredNumBackBuffers, TArray<VkImage>& OutImages, bool LockToVSync,
                     VulkanSwapChainRecreateInfo* RecreateInfo);
    void SetName(std::string_view InName) override;

    void Destroy(VulkanSwapChainRecreateInfo* RecreateInfo);

    EStatus Present(FVulkanQueue* PresentQueue, Ref<RSemaphore>& RenderingComplete);

    VkFormat GetFormat() const
    {
        return ImageFormat;
    }

    VkSwapchainKHR GetHandle() const
    {
        return SwapChain;
    }

    inline bool DoesLockToVSync() const
    {
        return LockToVSync;
    }

    UVector2 GetInternalSize() const
    {
        return InternalSize;
    }

private:
    int32 AcquireImageIndex(Ref<RSemaphore>& OutSemaphore);

private:
    int32 CurrentImageIndex;
    int32 SemaphoreIndex;

    bool LockToVSync;

    UVector2 InternalSize;

    VkFormat ImageFormat = VK_FORMAT_UNDEFINED;
    VkSwapchainKHR SwapChain;
    VkSurfaceKHR Surface;
    VkInstance Instance;

    TArray<Ref<RSemaphore>> ImageAcquiredSemaphore;
    TArray<Ref<RFence>> ImageInUseFence;

    friend class RVulkanViewport;
};

}    // namespace VulkanRHI

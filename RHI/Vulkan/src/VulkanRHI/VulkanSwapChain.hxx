#pragma once

#include "VulkanRHI/VulkanLoader.hxx"

namespace VulkanRHI
{

class VulkanTexture;
class VulkanDevice;
class Semaphore;
class Fence;
class VulkanQueue;

struct VulkanSwapChainRecreateInfo {
    VkSwapchainKHR SwapChain = VK_NULL_HANDLE;
    VkSurfaceKHR Surface = VK_NULL_HANDLE;
};

class VulkanSwapChain : public RObject
{
public:
    enum class Status {
        Healty = 0,
        OutOfDate = -1,
        SurfaceLost = -2,
    };

private:
    /// @brief Helper class to gather all information required for swapchain creation
    class SupportDetails
    {
    public:
        /// Gather swapchain support information
        static SupportDetails QuerySwapChainSupport(const Ref<VulkanDevice> &device, const VkSurfaceKHR &surface);

    public:
        /// Choose a fitting format
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat() const noexcept;
        /// Choose a presentation mode
        VkPresentModeKHR ChooseSwapPresentMode(bool LockToVSync) const noexcept;
        /// Check if the size if supported
        VkExtent2D ChooseSwapExtent(const glm::uvec2 &size) const noexcept;

    public:
        /// surface capability
        VkSurfaceCapabilitiesKHR Capabilities;
        /// List of supported format
        std::vector<VkSurfaceFormatKHR> Formats;
        /// List of supported presentation modes
        std::vector<VkPresentModeKHR> PresentModes;
    };

public:
    VulkanSwapChain(VkInstance InInstance, Ref<VulkanDevice> &InDevice, void *WindowHandle, glm::uvec2 Size,
                    uint32 InOutDesiredNumBackBuffers, std::vector<VkImage> &OutImages, bool LockToVSync,
                    VulkanSwapChainRecreateInfo *RecreateInfo);

    void Destroy(VulkanSwapChainRecreateInfo *RecreateInfo);

    Status Present(Ref<VulkanQueue> &PresentQueue, Ref<Semaphore> &RenderingComplete);

    VkFormat GetFormat() const { return ImageFormat; }

    VkSwapchainKHR GetHandle() const { return SwapChain; }

    inline bool DoesLockToVSync() const { return LockToVSync; }

    void SetName(std::string_view InName) override;

private:
    int32 AcquireImageIndex(Ref<Semaphore> &OutSemaphore);

private:
    Ref<VulkanDevice> Device;
    int32 CurrentImageIndex;
    int32 SemaphoreIndex;

    bool LockToVSync;

    glm::uvec2 InternalSize;

    VkFormat ImageFormat = VK_FORMAT_UNDEFINED;
    VkSwapchainKHR SwapChain;
    VkSurfaceKHR Surface;
    VkInstance Instance;

    std::vector<Ref<Semaphore>> ImageAcquiredSemaphore;
    std::vector<Ref<Fence>> ImageInUseFence;

    friend class VulkanViewport;
};

}    // namespace VulkanRHI

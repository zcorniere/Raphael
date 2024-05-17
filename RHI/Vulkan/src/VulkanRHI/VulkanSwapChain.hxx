#pragma once

#include "VulkanRHI/VulkanLoader.hxx"

class Window;

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

class VulkanSwapChain : public RObject, public IDeviceChild
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
        static SupportDetails QuerySwapChainSupport(const VulkanDevice* Device, const VkSurfaceKHR& Surface);

    public:
        /// Choose a fitting format
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat() const noexcept;
        /// Choose a presentation mode
        VkPresentModeKHR ChooseSwapPresentMode(bool LockToVSync) const noexcept;
        /// Check if the size if supported
        VkExtent2D ChooseSwapExtent(const glm::uvec2& InSize) const noexcept;

    public:
        /// surface capability
        VkSurfaceCapabilitiesKHR Capabilities;
        /// List of supported format
        Array<VkSurfaceFormatKHR> Formats;
        /// List of supported presentation modes
        Array<VkPresentModeKHR> PresentModes;
    };

public:
    VulkanSwapChain(VkInstance InInstance, VulkanDevice* InDevice, const glm::uvec2& InSize, Window* WindowHandle,
                    uint32 InOutDesiredNumBackBuffers, Array<VkImage>& OutImages, bool LockToVSync,
                    VulkanSwapChainRecreateInfo* RecreateInfo);
    void SetName(std::string_view InName) override;

    void Destroy(VulkanSwapChainRecreateInfo* RecreateInfo);

    Status Present(VulkanQueue* PresentQueue, Ref<Semaphore>& RenderingComplete);

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

    glm::uvec2 GetInternalSize() const
    {
        return InternalSize;
    }

private:
    int32 AcquireImageIndex(Ref<Semaphore>& OutSemaphore);

private:
    int32 CurrentImageIndex;
    int32 SemaphoreIndex;

    bool LockToVSync;

    glm::uvec2 InternalSize;

    VkFormat ImageFormat = VK_FORMAT_UNDEFINED;
    VkSwapchainKHR SwapChain;
    VkSurfaceKHR Surface;
    VkInstance Instance;

    Array<Ref<Semaphore>> ImageAcquiredSemaphore;
    Array<Ref<Fence>> ImageInUseFence;

    friend class VulkanViewport;
};

}    // namespace VulkanRHI

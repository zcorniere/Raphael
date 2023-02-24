#pragma once

#include "Engine/Renderer/RHI/RHIResource.hxx"

#include "Engine/Renderer/Vulkan/VulkanMemoryManager.hxx"
#include "Engine/Renderer/Vulkan/VulkanSwapChain.hxx"

namespace VulkanRHI
{

class Semaphore;
class VulkanDevice;

inline VkImageViewType TextureDimensionToVkImageViewType(TextureDimension Dimension)
{
    (void)Dimension;
    // Only support 2D for now
    return VK_IMAGE_VIEW_TYPE_2D;
}

class VulkanTexture : public RHITexture
{
public:
    VulkanTexture(Ref<VulkanDevice> InDevice, const RHITextureCreateDesc &InDesc);
    virtual ~VulkanTexture();

    void SetName(std::string_view InName) override;

    void *GetNativeResource() const override
    {
        return Image;
    }

    VkImageViewType GetViewType() const
    {
        return TextureDimensionToVkImageViewType(Description.Dimension);
    }

private:
    RHITextureCreateDesc Description;
    Ref<VulkanDevice> Device;
    Ref<VulkanMemoryAllocation> Allocation;
    VkMemoryRequirements MemoryRequirements;
    VkFormat Format;
    VkImage Image;
};

struct VulkanTextureView {
    VulkanTextureView(): View(VK_NULL_HANDLE), Image(VK_NULL_HANDLE)
    {
    }

    void Create(Ref<VulkanDevice> &Device, VkImage InImage, VkImageViewType ViewType, VkImageAspectFlags AspectFlags,
                VkFormat Format, uint32 FirstMip, uint32 NumMips);
    void Destroy(Ref<VulkanDevice> &Device);

    VkImageView View;
    VkImage Image;
};

class VulkanShader : public RHIShader
{
public:
    VulkanShader(RHIShaderType Type): RHIShader(Type)
    {
    }
};

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

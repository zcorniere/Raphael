#pragma once

#include "Engine/Renderer/RHI/RHIResource.hxx"

#include "Engine/Renderer/Vulkan/VulkanLoader.hxx"

namespace VulkanRHI
{

class VulkanDevice;
class VulkanMemoryAllocation;

inline VkImageViewType TextureDimensionToVkImageViewType(ETextureDimension Dimension)
{
    (void)Dimension;
    // Only support 2D for now
    return VK_IMAGE_VIEW_TYPE_2D;
}

inline VkImageType TextureDimensionToVkImageType(ETextureDimension Dimension)
{
    (void)Dimension;
    // Only support 2D for now
    return VK_IMAGE_TYPE_2D;
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

}    // namespace VulkanRHI

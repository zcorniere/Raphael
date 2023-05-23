#pragma once

#include "Engine/Core/RHI/RHIDefinitions.hxx"
#include "Engine/Core/RHI/Resources/RHITexture.hxx"

#include "VulkanRHI/VulkanLoader.hxx"

namespace VulkanRHI
{

class VulkanDevice;
class VulkanMemoryAllocation;

inline VkFormat TextureFormatToVkFormat(EImageFormat Format)
{
    switch (Format) {
        case EImageFormat::R8G8B8_SRGB:
            return VK_FORMAT_R8G8B8_SRGB;
        case EImageFormat::R8G8B8A8_RGBA:
            return VK_FORMAT_R8G8B8A8_SRGB;
    }
    checkNoEntry();
}

inline VkImageViewType TextureDimensionToVkImageViewType(EImageDimension Dimension)
{
    switch (Dimension) {
        case EImageDimension::Texture2D:
            return VK_IMAGE_VIEW_TYPE_2D;
    }
    checkNoEntry();
}

inline VkImageType TextureDimensionToVkImageType(EImageDimension Dimension)
{
    switch (Dimension) {
        case EImageDimension::Texture2D:
            return VK_IMAGE_TYPE_2D;
        default:
            checkNoEntry();
    }
    checkNoEntry();
}

class VulkanTexture : public RHITexture
{
public:
    VulkanTexture(Ref<VulkanDevice> InDevice, const RHITextureCreateDesc& InDesc);
    virtual ~VulkanTexture();

    void SetName(std::string_view InName) override;

    void* GetNativeResource() const override
    {
        return (void*)Image;
    }

    VkImageViewType GetViewType() const
    {
        return TextureDimensionToVkImageViewType(Description.Dimension);
    }
    VkImageLayout GetLayout() const
    {
        return Layout;
    }

private:
    RHITextureCreateDesc Description;
    Ref<VulkanDevice> Device;
    Ref<VulkanMemoryAllocation> Allocation;
    VkMemoryRequirements MemoryRequirements;
    VkImage Image;
    VkImageLayout Layout;
};

struct VulkanTextureView {
    VulkanTextureView(): View(VK_NULL_HANDLE), Image(VK_NULL_HANDLE)
    {
    }

    void Create(Ref<VulkanDevice>& Device, VkImage InImage, VkImageViewType ViewType, VkImageAspectFlags AspectFlags,
                VkFormat Format, uint32 FirstMip, uint32 NumMips);
    void Destroy(Ref<VulkanDevice>& Device);

    VkImageView View;
    VkImage Image;
};

}    // namespace VulkanRHI

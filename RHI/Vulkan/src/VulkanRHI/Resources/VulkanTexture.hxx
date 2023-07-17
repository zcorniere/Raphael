#pragma once

#include "Engine/Core/RHI/RHIDefinitions.hxx"
#include "Engine/Core/RHI/Resources/RHITexture.hxx"

#include "VulkanRHI/VulkanLoader.hxx"

namespace VulkanRHI
{

class VulkanDevice;
class VulkanMemoryAllocation;

class VulkanTexture : public RHITexture
{
public:
    VulkanTexture(VulkanDevice* InDevice, const RHITextureCreateDesc& InDesc);
    virtual ~VulkanTexture();

    void SetName(std::string_view InName) override;

    VkImageView GetImageView() const;
    VkImageViewType GetViewType() const;
    VkImageLayout GetLayout() const;

private:
    VulkanDevice* Device;

    RHITextureCreateDesc Description;
    Ref<VulkanMemoryAllocation> Allocation;
    VkMemoryRequirements MemoryRequirements;
    VkImage Image;
    VkImageLayout Layout;
    mutable VkImageView View;
};

struct VulkanTextureView {
    VulkanTextureView(): View(VK_NULL_HANDLE), Image(VK_NULL_HANDLE)
    {
    }

    void Create(VulkanDevice* Device, VkImage InImage, VkImageViewType ViewType, VkImageAspectFlags AspectFlags,
                VkFormat Format, uint32 FirstMip, uint32 NumMips);
    void Destroy(VulkanDevice* Device);

    VkImageView View;
    VkImage Image;
};

}    // namespace VulkanRHI

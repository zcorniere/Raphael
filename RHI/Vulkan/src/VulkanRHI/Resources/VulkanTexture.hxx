#pragma once

#include "Engine/Core/RHI/RHIDefinitions.hxx"
#include "Engine/Core/RHI/Resources/RHITexture.hxx"

namespace VulkanRHI
{

class FVulkanDevice;
class FVulkanCmdBuffer;
class RVulkanMemoryAllocation;

class RVulkanTexture : public RRHITexture, public IDeviceChild
{
    RTTI_DECLARE_TYPEINFO(RVulkanTexture, RRHITexture);

public:
    RVulkanTexture(FVulkanDevice* InDevice, const FRHITextureSpecification& InDesc);
    virtual ~RVulkanTexture();

    virtual void SetName(std::string_view InName) override;

    virtual void Invalidate() override;

    VkImage GetImage() const;
    VkImageView GetImageView() const;
    VkImageViewType GetViewType() const;

    VkImageLayout GetLayout() const;
    void SetLayout(FVulkanCmdBuffer* CommandBuffer, VkImageLayout NewLayout);

    VkImageLayout GetDefaultLayout() const;

private:
    void CreateTexture();
    void DestroyTexture();

private:
    Ref<RVulkanMemoryAllocation> Allocation;
    VkMemoryRequirements MemoryRequirements;
    VkImage Image = VK_NULL_HANDLE;
    VkImageLayout Layout = VK_IMAGE_LAYOUT_UNDEFINED;
    mutable VkImageView View = VK_NULL_HANDLE;
};

struct VulkanTextureView
{
    VulkanTextureView(): View(VK_NULL_HANDLE), Image(VK_NULL_HANDLE)
    {
    }

    void Create(FVulkanDevice* Device, VkImage InImage, VkImageViewType ViewType, VkImageAspectFlags AspectFlags,
                VkFormat Format, uint32 FirstMip, uint32 NumMips);
    void Destroy(FVulkanDevice* Device);

    VkImageView View;
    VkImage Image;
};

}    // namespace VulkanRHI

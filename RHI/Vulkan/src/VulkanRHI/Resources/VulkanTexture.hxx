#pragma once

#include "Engine/Core/RHI/RHIDefinitions.hxx"
#include "Engine/Core/RHI/Resources/RHITexture.hxx"

namespace VulkanRHI
{

class VulkanDevice;
class VulkanCmdBuffer;
class VulkanMemoryAllocation;

class VulkanTexture : public RHITexture, public IDeviceChild
{
public:
    VulkanTexture(VulkanDevice* InDevice, const RHITextureSpecification& InDesc);
    virtual ~VulkanTexture();

    virtual void SetName(std::string_view InName) override;

    virtual void Invalidate() override;

    VkImage GetImage() const;
    VkImageView GetImageView() const;
    VkImageViewType GetViewType() const;

    VkImageLayout GetLayout() const;
    void SetLayout(VkImageLayout NewLayout);
    void SetLayout(VulkanCmdBuffer* CommandBuffer, VkImageLayout NewLayout);

private:
    void CreateTexture();
    void DestroyTexture();

private:
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

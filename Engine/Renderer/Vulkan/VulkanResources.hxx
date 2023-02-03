#include "Engine/Renderer/RHI/RHIResource.hxx"

#include "Engine/Renderer/Vulkan/VulkanMemoryManager.hxx"

namespace Raphael::RHI
{

class VulkanDevice;

VkImageViewType TextureDimensionToVkImageViewType(TextureDimension Dimension)
{
    (void)Dimension;
    // Only support 2D for now
    return VK_IMAGE_VIEW_TYPE_2D;
}

class VulkanTexture : public RHITexture
{
public:
    VulkanTexture(Ref<VulkanDevice> InDevice, const RHITextureCreateDesc &InDesc);

    void *GetNativeResource() const override
    {
        return Image;
    }

private:
    Ref<VulkanDevice> Device;
    Ref<VulkanMemoryAllocation> Allocation;
    VkMemoryRequirements MemoryRequirements;
    VkFormat Format;
    VkImage Image;
};

}    // namespace Raphael::RHI

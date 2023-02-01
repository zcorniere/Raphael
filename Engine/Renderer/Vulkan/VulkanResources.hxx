#include "Engine/Renderer/RHI/RHIResource.hxx"

#include "Engine/Renderer/Vulkan/VulkanMemoryManager.hxx"

namespace Raphael::RHI
{

class VulkanDevice;

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

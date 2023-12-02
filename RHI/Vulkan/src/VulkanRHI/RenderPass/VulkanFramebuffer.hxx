#pragma once

#include "Engine/Core/RHI/RHIDefinitions.hxx"

#include "VulkanRHI/Resources/VulkanTexture.hxx"

namespace VulkanRHI
{

class VulkanDevice;
class VulkanRenderPass;

class VulkanFramebuffer : public RObject, public IDeviceChild
{
public:
    VulkanFramebuffer(VulkanDevice* const InDevice, const VulkanRenderPass* const InRenderPass,
                      const RHIFramebufferDefinition& InDefinitions);
    ~VulkanFramebuffer();

    void SetName(std::string_view InName) override;

    FORCEINLINE bool HasDepthTarget() const
    {
        return Definition.DepthTarget.IsValid();
    }
    FORCEINLINE unsigned HasResolveTargets() const
    {
        return Definition.ResolveTarget.Size();
    }
    VkFramebuffer GetFramebuffer() const
    {
        return Framebuffer;
    }
    const RHIFramebufferDefinition& GetDefinition() const
    {
        return Definition;
    }

    glm::uvec2 GetExtent() const
    {
        return Definition.Extent;
    }

    glm::uvec2 GetOffset() const
    {
        return Definition.Offset;
    }

private:
    VkFramebuffer CreateFrameBuffer();
    void DestroyFrameBuffer();
    Array<VkImageView> GetFramebufferAttachment(const Array<Ref<RHITexture>>& SourceTextures);

private:
    const VulkanRenderPass* const RenderPass;

    RHIFramebufferDefinition Definition;
    VkFramebuffer Framebuffer;
};
}    // namespace VulkanRHI

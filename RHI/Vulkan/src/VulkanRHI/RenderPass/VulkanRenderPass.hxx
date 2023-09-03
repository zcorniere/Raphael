#pragma once

#include "Engine/Core/RHI/RHIDefinitions.hxx"

#include "VulkanRHI/Resources/VulkanTexture.hxx"
#include "VulkanRHI/VulkanLoader.hxx"

namespace VulkanRHI
{

class VulkanDevice;
class VulkanCmdBuffer;

class VulkanRenderPass : public RObject
{
private:
    struct AttachmentRefs {
        Array<VkAttachmentReference> ColorReference;
        Array<VkAttachmentReference> DepthReference;
        Array<VkAttachmentReference> ResolveReference;
    };

public:
    VulkanRenderPass(VulkanDevice* InDevice, const RHIRenderPassDescription& InDescription,
                     VkRenderPass ExternalPass = VK_NULL_HANDLE);
    ~VulkanRenderPass();

    void Begin(VulkanCmdBuffer* CmdBuffer, const VkRect2D RenderArea);
    void End(VulkanCmdBuffer* CmdBuffer);

    FORCEINLINE bool HasDepthTarget() const
    {
        return Description.DepthTarget.IsValid();
    }
    unsigned HasResolveTargets() const
    {
        return Description.ResolveTarget.Size();
    }

    VkRenderPass GetRenderPass() const
    {
        return RenderPass;
    }
    VkFramebuffer GetFramebuffer() const
    {
        return FrameBuffer;
    }

    glm::uvec2 GetExtent() const
    {
        return Description.Size;
    }
    glm::uvec2 GetOffset() const
    {
        return Description.Offset;
    }

    void SetName(std::string_view InName) override;

private:
    VkFramebuffer CreateFrameBuffer();
    Array<VkImageView> GetFramebufferAttachment(const Array<Ref<RHITexture>>& SourceTextures);

    VkRenderPass CreateRenderPass();
    AttachmentRefs FillAttachmentReference(const Array<VkAttachmentDescription>& Targets);
    VkAttachmentDescription GetAttachmentDescription(const Ref<RHITexture>& Target, ETextureUsageFlags Flags) const;
    Array<VkAttachmentDescription> GetAttachmentDescriptions(const Array<Ref<RHITexture>>& Targets,
                                                             ETextureUsageFlags Flags);

private:
    bool bHasBegun = false;
    VulkanDevice* Device;

    RHIRenderPassDescription Description;
    VkRenderPass RenderPass;
    VkFramebuffer FrameBuffer;
};

}    // namespace VulkanRHI

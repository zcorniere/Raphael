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
    VulkanRenderPass(Ref<VulkanDevice>& InDevice, const RHIRenderPassDescription& InDescription);
    ~VulkanRenderPass();

    void Begin(Ref<VulkanCmdBuffer>& CmdBuffer);
    void End(Ref<VulkanCmdBuffer>& CmdBuffer);

    VkRenderPass GetRenderPass() const
    {
        return RenderPass;
    }
    VkFramebuffer GetFramebuffer() const
    {
        return FrameBuffer;
    }

private:
    VkFramebuffer CreateFrameBuffer();
    Array<VkImageView> GetFramebufferAttachment(const Array<Ref<VulkanTexture>>& SourceTextures);

    VkRenderPass CreateRenderPass();
    AttachmentRefs FillAttachmentReference(const Array<VkAttachmentDescription>& Targets);
    VkAttachmentDescription GetAttachmentDescription(const RHIRenderPassDescription::RenderingTargetInfo& Target,
                                                     ETextureCreateFlags Flags) const;
    Array<VkAttachmentDescription>
    GetAttachmentDescriptions(const Array<RHIRenderPassDescription::RenderingTargetInfo>& Targets,
                              ETextureCreateFlags Flags);

    Ref<VulkanTexture> CreateFramebufferTextures(const RHIRenderPassDescription::RenderingTargetInfo& TargetInfo,
                                                 ETextureCreateFlags Flags);

private:
    Ref<VulkanDevice> Device;

    Array<Ref<VulkanTexture>> ColorTarget;
    Array<Ref<VulkanTexture>> ResolveTarget;
    Ref<VulkanTexture> DepthTarget;

    RHIRenderPassDescription Description;
    VkRenderPass RenderPass;
    VkFramebuffer FrameBuffer;
};

}    // namespace VulkanRHI
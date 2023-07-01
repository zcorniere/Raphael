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
    VulkanRenderPass(Ref<VulkanDevice>& InDevice, const RHIRenderPassDescription& InDescription,
                     VkRenderPass ExternalPass = VK_NULL_HANDLE);
    ~VulkanRenderPass();

    void Begin(Ref<VulkanCmdBuffer>& CmdBuffer, const VkRect2D RenderArea);
    void End(Ref<VulkanCmdBuffer>& CmdBuffer);

    VkRenderPass GetRenderPass() const
    {
        return RenderPass;
    }
    VkFramebuffer GetFramebuffer() const
    {
        return FrameBuffer;
    }

    void SetName(std::string_view InName) override;

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
    bool bHasBegun = false;
    Ref<VulkanDevice> Device;

    Array<Ref<VulkanTexture>> ColorTarget;
    Array<Ref<VulkanTexture>> ResolveTarget;
    Ref<VulkanTexture> DepthTarget;

    RHIRenderPassDescription Description;
    VkRenderPass RenderPass;
    VkFramebuffer FrameBuffer;
};

}    // namespace VulkanRHI

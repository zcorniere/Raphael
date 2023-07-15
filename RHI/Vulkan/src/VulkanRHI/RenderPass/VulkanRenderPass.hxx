#pragma once

#include "Engine/Core/RHI/RHIDefinitions.hxx"

#include "VulkanRHI/Resources/VulkanTexture.hxx"
#include "VulkanRHI/VulkanLoader.hxx"

namespace VulkanRHI
{

class VulkanDevice;
class VulkanCmdBuffer;

class VulkanRenderPass : public RefCounted, public NamedClassWithTypeName<VulkanRenderPass>
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

    VkRenderPass GetRenderPass() const
    {
        return RenderPass;
    }
    VkFramebuffer GetFramebuffer() const
    {
        return FrameBuffer;
    }

    virtual void SetName(std::string_view InName) override;

private:
    VkFramebuffer CreateFrameBuffer();
    Array<VkImageView> GetFramebufferAttachment(const Array<VulkanTexture*>& SourceTextures);

    VkRenderPass CreateRenderPass();
    AttachmentRefs FillAttachmentReference(const Array<VkAttachmentDescription>& Targets);
    VkAttachmentDescription GetAttachmentDescription(const RHIRenderPassDescription::RenderingTargetInfo& Target,
                                                     ETextureCreateFlags Flags) const;
    Array<VkAttachmentDescription>
    GetAttachmentDescriptions(const Array<RHIRenderPassDescription::RenderingTargetInfo>& Targets,
                              ETextureCreateFlags Flags);

    VulkanTexture* CreateFramebufferTextures(const RHIRenderPassDescription::RenderingTargetInfo& TargetInfo,
                                             ETextureCreateFlags Flags);

private:
    bool bHasBegun = false;
    VulkanDevice* Device;

    Array<VulkanTexture*> ColorTarget;
    Array<VulkanTexture*> ResolveTarget;
    VulkanTexture* DepthTarget;

    RHIRenderPassDescription Description;
    VkRenderPass RenderPass;
    VkFramebuffer FrameBuffer;
};

}    // namespace VulkanRHI

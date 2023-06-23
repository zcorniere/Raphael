#pragma once

#include "Engine/Core/RHI/RHIDefinitions.hxx"

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
    AttachmentRefs FillAttachmentReference(const Array<VkAttachmentDescription>& Targets);

    VkAttachmentDescription GetAttachmentDescription(const RHIRenderPassDescription::RenderingTarget& Target);

    Array<VkAttachmentDescription>
    GetAttachmentDescriptions(const Array<RHIRenderPassDescription::RenderingTarget>& Targets);

private:
    Ref<VulkanDevice> Device;

    RHIRenderPassDescription Description;
    VkRenderPass RenderPass;
    VkFramebuffer FrameBuffer;
};

}    // namespace VulkanRHI

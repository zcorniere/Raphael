#pragma once

#include "Engine/Core/RHI/RHIDefinitions.hxx"

#include "VulkanRHI/RenderPass/VulkanFramebuffer.hxx"
#include "VulkanRHI/Resources/VulkanTexture.hxx"
#include "VulkanRHI/VulkanLoader.hxx"

namespace VulkanRHI
{

class VulkanDevice;
class VulkanCmdBuffer;

class VulkanRenderPass : public RObject, public IDeviceChild
{
private:
    struct AttachmentRefs {
        Array<VkAttachmentReference> ColorReference;
        Array<VkAttachmentReference> DepthReference;
        Array<VkAttachmentReference> ResolveReference;
    };

public:
    VulkanRenderPass(VulkanDevice* const InDevice, const RHIRenderPassDescription& InDescription,
                     VkRenderPass ExternalPass = VK_NULL_HANDLE);
    ~VulkanRenderPass();

    void Begin(VulkanCmdBuffer* const CmdBuffer, const VulkanFramebuffer* const Framebuffer, const VkRect2D RenderArea);
    void End(VulkanCmdBuffer* const CmdBuffer);

    FORCEINLINE bool HasDepthTarget() const
    {
        return Description.DepthTarget.has_value();
    }
    FORCEINLINE unsigned HasResolveTargets() const
    {
        return Description.ResolveTarget.Size();
    }

    FORCEINLINE VkRenderPass GetRenderPass() const
    {
        return RenderPass;
    }

    void SetName(std::string_view InName) override;

private:
    VkRenderPass CreateRenderPass();
    AttachmentRefs FillAttachmentReference(const Array<VkAttachmentDescription>& Targets);
    VkAttachmentDescription GetAttachmentDescription(const RHIRenderPassTarget& Target) const;
    Array<VkAttachmentDescription> GetAttachmentDescriptions(const Array<RHIRenderPassTarget>& Targets);

private:
    bool bHasBegun = false;

    RHIRenderPassDescription Description;
    VkRenderPass RenderPass;
};

}    // namespace VulkanRHI

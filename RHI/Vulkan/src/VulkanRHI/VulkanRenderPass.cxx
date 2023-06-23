#include "VulkanRHI/VulkanRenderPass.hxx"

#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanUtils.hxx"

namespace VulkanRHI
{

VulkanRenderPass::VulkanRenderPass(Ref<VulkanDevice>& InDevice, const RHIRenderPassDescription& InDescription)
    : Device(InDevice), Description(InDescription), RenderPass(VK_NULL_HANDLE), FrameBuffer(VK_NULL_HANDLE)
{
    Array<VkAttachmentDescription> Attachments = GetAttachmentDescriptions(Description.ColorTarget);
    Attachments.Append(GetAttachmentDescriptions(Description.ResolveTarget));
    if (Description.DepthTarget.TargetTexture)
        Attachments.Append(GetAttachmentDescriptions({Description.DepthTarget}));

    AttachmentRefs AttachRef = FillAttachmentReference(Attachments);

    VkSubpassDescription Subpass{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = AttachRef.ColorReference.Size(),
        .pColorAttachments = AttachRef.ColorReference.Raw(),
        .pResolveAttachments = AttachRef.ResolveReference.Raw(),
        .pDepthStencilAttachment = AttachRef.DepthReference.Raw(),
    };
    VkSubpassDependency Dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = VK_ACCESS_NONE,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    };

    VkRenderPassCreateInfo RenderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = Attachments.Size(),
        .pAttachments = Attachments.Raw(),
        .subpassCount = 1,
        .pSubpasses = &Subpass,
        .dependencyCount = 1,
        .pDependencies = &Dependency,
    };
    VK_CHECK_RESULT(VulkanAPI::vkCreateRenderPass(Device->GetHandle(), &RenderPassInfo, nullptr, &RenderPass));
}

VulkanRenderPass::~VulkanRenderPass()
{
    if (RenderPass) {
        VulkanAPI::vkDestroyRenderPass(Device->GetHandle(), RenderPass, nullptr);
    }

    if (FrameBuffer) {
        VulkanAPI::vkDestroyFramebuffer(Device->GetHandle(), FrameBuffer, nullptr);
    }
}

void VulkanRenderPass::Begin(Ref<VulkanCmdBuffer>& CmdBuffer)
{
}
void VulkanRenderPass::End(Ref<VulkanCmdBuffer>& CmdBuffer)
{
}

static VkImageLayout GetLayout(ETextureCreateFlags Flags)
{
    switch (Flags) {
        case ETextureCreateFlags::RenderTargetable:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case ETextureCreateFlags::ResolveTargetable:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        case ETextureCreateFlags::DepthStencilTargetable:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        default:
            break;
    }
    checkNoEntry();
    return VK_IMAGE_LAYOUT_UNDEFINED;
}

VulkanRenderPass::AttachmentRefs
VulkanRenderPass::FillAttachmentReference(const Array<VkAttachmentDescription>& Targets)
{
    AttachmentRefs Ref;

    for (unsigned i = 0; i < Targets.Size(); i++) {
        VkImageLayout Layout = Targets[i].finalLayout;

        VkAttachmentReference AttachRef{
            .attachment = i,
            .layout = Layout,
        };
        switch (Layout) {
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                Ref.ColorReference.Add(AttachRef);
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
                Ref.DepthReference.Add(AttachRef);
                break;
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                AttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                Ref.ResolveReference.Add(AttachRef);
                break;
            default:
                checkNoEntry();
        };
    }
    return Ref;
}

VkAttachmentDescription
VulkanRenderPass::GetAttachmentDescription(const RHIRenderPassDescription::RenderingTarget& Target)
{
    const RHITextureCreateDesc& TextureDesc = Target.TargetTexture->GetDescription();
    VkAttachmentDescription Description{
        .format = ImageFormatToFormat(TextureDesc.Format),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = GetLayout(TextureDesc.Flags),
    };
    return Description;
}

Array<VkAttachmentDescription>
VulkanRenderPass::GetAttachmentDescriptions(const Array<RHIRenderPassDescription::RenderingTarget>& Targets)
{
    Array<VkAttachmentDescription> AttachmentDescription;
    AttachmentDescription.Reserve(Targets.Size());
    for (const RHIRenderPassDescription::RenderingTarget& Target: Targets) {
        AttachmentDescription.Add(GetAttachmentDescription(Target));
    }

    return AttachmentDescription;
}
}    // namespace VulkanRHI

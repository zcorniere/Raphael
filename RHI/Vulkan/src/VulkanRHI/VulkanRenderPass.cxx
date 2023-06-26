#include "VulkanRHI/VulkanRenderPass.hxx"

#include "VulkanRHI/Resources/VulkanTexture.hxx"
#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanUtils.hxx"

namespace VulkanRHI
{

VulkanRenderPass::VulkanRenderPass(Ref<VulkanDevice>& InDevice, const RHIRenderPassDescription& InDescription)
    : Device(InDevice), Description(InDescription), RenderPass(VK_NULL_HANDLE), FrameBuffer(VK_NULL_HANDLE)
{
    CreateRenderPass();

    ColorTarget.Reserve(InDescription.ColorTarget.Size());
    for (const RHIRenderPassDescription::RenderingTargetInfo& Info: InDescription.ColorTarget) {
        ColorTarget.Add(CreateFramebufferTextures(Info, ETextureCreateFlags::RenderTargetable));
    }

    ResolveTarget.Reserve(InDescription.ResolveTarget.Size());
    for (const RHIRenderPassDescription::RenderingTargetInfo& Info: InDescription.ResolveTarget) {
        ResolveTarget.Add(CreateFramebufferTextures(Info, ETextureCreateFlags::ResolveTargetable));
    }

    if (InDescription.DepthTarget) {
        DepthTarget =
            CreateFramebufferTextures(InDescription.DepthTarget.value(), ETextureCreateFlags::DepthStencilTargetable);
    }

    CreateFrameBuffer();
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

VkFramebuffer VulkanRenderPass::CreateFrameBuffer()
{
    Array<VkImageView> Attachments(GetFramebufferAttachment(ColorTarget));
    Attachments.Append(GetFramebufferAttachment({DepthTarget}));
    Attachments.Append(GetFramebufferAttachment(ResolveTarget));

    VkFramebufferCreateInfo CreateInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = GetRenderPass(),
        .attachmentCount = Attachments.Size(),
        .pAttachments = Attachments.Raw(),
        .width = Description.RenderPassSize.x,
        .height = Description.RenderPassSize.y,
        .layers = 1,
    };
    VK_CHECK_RESULT(VulkanAPI::vkCreateFramebuffer(Device->GetHandle(), &CreateInfo, nullptr, &FrameBuffer));
    VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_FRAMEBUFFER, FrameBuffer, "%s [Framebuffer]", GetName());
    return FrameBuffer;
}

Array<VkImageView> VulkanRenderPass::GetFramebufferAttachment(const Array<Ref<VulkanTexture>>& SourceTextures)
{
    Array<VkImageView> ImageView;
    ImageView.Reserve(SourceTextures.Size());
    for (const Ref<VulkanTexture>& Texture: SourceTextures) {
        ImageView.Add(Texture->GetImageView());
    }
    return ImageView;
}

VkRenderPass VulkanRenderPass::CreateRenderPass()
{
    Array<VkAttachmentDescription> Attachments =
        GetAttachmentDescriptions(Description.ColorTarget, ETextureCreateFlags::RenderTargetable);
    Attachments.Append(GetAttachmentDescriptions(Description.ResolveTarget, ETextureCreateFlags::ResolveTargetable));
    if (Description.DepthTarget)
        Attachments.Append(
            GetAttachmentDescriptions({Description.DepthTarget.value()}, ETextureCreateFlags::DepthStencilTargetable));

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
    return RenderPass;
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
VulkanRenderPass::GetAttachmentDescription(const RHIRenderPassDescription::RenderingTargetInfo& Target,
                                           ETextureCreateFlags Flags) const
{
    VkAttachmentDescription Description{
        .format = ImageFormatToFormat(Target.Format),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = GetLayout(Flags),
    };
    return Description;
}

Array<VkAttachmentDescription>
VulkanRenderPass::GetAttachmentDescriptions(const Array<RHIRenderPassDescription::RenderingTargetInfo>& Targets,
                                            ETextureCreateFlags Flags)
{
    Array<VkAttachmentDescription> AttachmentDescription;
    AttachmentDescription.Reserve(Targets.Size());
    for (const RHIRenderPassDescription::RenderingTargetInfo& Target: Targets) {
        AttachmentDescription.Add(GetAttachmentDescription(Target, Flags));
    }

    return AttachmentDescription;
}
Ref<VulkanTexture>
VulkanRenderPass::CreateFramebufferTextures(const RHIRenderPassDescription::RenderingTargetInfo& TargetInfo,
                                            ETextureCreateFlags Flags)
{
    RHITextureCreateDesc TextureCreate{
        .Flags = Flags,
        .Dimension = EImageDimension::Texture2D,
        .Format = TargetInfo.Format,
        .Extent = Description.RenderPassSize,
    };
    return RHI::CreateTexture(TextureCreate);
}
}    // namespace VulkanRHI

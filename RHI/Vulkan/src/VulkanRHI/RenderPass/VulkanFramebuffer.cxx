#include "VulkanRHI/RenderPass/VulkanFramebuffer.hxx"

#include "VulkanRHI/RenderPass/VulkanRenderPass.hxx"
#include "VulkanRHI/VulkanDevice.hxx"

namespace VulkanRHI
{

VulkanFramebuffer::VulkanFramebuffer(VulkanDevice* const InDevice, const VulkanRenderPass* const InRenderPass,
                                     const RHIFramebufferDefinition& InDefinitions)
    : Device(InDevice), RenderPass(InRenderPass), Definition(InDefinitions)
{
    CreateFrameBuffer();
}

VulkanFramebuffer::~VulkanFramebuffer()
{
    DestroyFrameBuffer();
}

void VulkanFramebuffer::SetName(std::string_view InName)
{
    RObject::SetName(InName);

    if (Framebuffer) {
        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_FRAMEBUFFER, Framebuffer, "{:s}.Framebuffer", GetName());
    }
}

VkFramebuffer VulkanFramebuffer::CreateFrameBuffer()
{
    Array<VkImageView> Attachments(GetFramebufferAttachment(Definition.ColorTarget));
    if (HasDepthTarget())
        Attachments.Append(GetFramebufferAttachment({
            Definition.DepthTarget,
        }));
    Attachments.Append(GetFramebufferAttachment(Definition.ResolveTarget));

    VkFramebufferCreateInfo CreateInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = RenderPass->GetRenderPass(),
        .attachmentCount = Attachments.Size(),
        .pAttachments = Attachments.Raw(),
        .width = Definition.Extent.x,
        .height = Definition.Extent.y,
        .layers = 1,
    };
    VK_CHECK_RESULT(
        VulkanAPI::vkCreateFramebuffer(Device->GetHandle(), &CreateInfo, VULKAN_CPU_ALLOCATOR, &Framebuffer));
    VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_FRAMEBUFFER, Framebuffer, "{:s}.Framebuffer", GetName());
    return Framebuffer;
}

void VulkanFramebuffer::DestroyFrameBuffer()
{
    if (Framebuffer) {
        VulkanAPI::vkDestroyFramebuffer(Device->GetHandle(), Framebuffer, VULKAN_CPU_ALLOCATOR);
        Framebuffer = VK_NULL_HANDLE;
    }
}

Array<VkImageView> VulkanFramebuffer::GetFramebufferAttachment(const Array<Ref<RHITexture>>& SourceTextures)
{
    Array<VkImageView> ImageView;
    ImageView.Reserve(SourceTextures.Size());
    for (const Ref<RHITexture>& Texture: SourceTextures) {
        WeakRef<VulkanTexture> VkTexture = Texture.As<VulkanTexture>();
        ImageView.Add(VkTexture->GetImageView());
    }
    return ImageView;
}

}    // namespace VulkanRHI

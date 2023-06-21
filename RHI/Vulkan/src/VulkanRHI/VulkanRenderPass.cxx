#include "VulkanRHI/VulkanRenderPass.hxx"

#include "VulkanRHI/VulkanDevice.hxx"

namespace VulkanRHI
{

VulkanRenderPass::VulkanRenderPass(Ref<VulkanDevice>& InDevice, const RHIRenderPassDescription& InDescription)
    : Device(InDevice), Description(InDescription)
{
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

}    // namespace VulkanRHI

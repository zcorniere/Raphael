#pragma once

#include "Engine/Core/RHI/RHIDefinitions.hxx"

#include "VulkanRHI/VulkanLoader.hxx"

namespace VulkanRHI
{

class VulkanDevice;

class VulkanRenderPass : public RObject
{
public:
    VulkanRenderPass(Ref<VulkanDevice>& InDevice, const RHIRenderPassDescription& InDescription);
    ~VulkanRenderPass();

    VkRenderPass GetRenderPass() const
    {
        return RenderPass;
    }
    VkFramebuffer GetFramebuffer() const
    {
        return FrameBuffer;
    }

private:
    Ref<VulkanDevice> Device;

    RHIRenderPassDescription Description;
    VkRenderPass RenderPass;
    VkFramebuffer FrameBuffer;
};

}    // namespace VulkanRHI

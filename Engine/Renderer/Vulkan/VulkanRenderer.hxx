#pragma once

#include "Engine/Renderer/RendererAPI.hxx"

#include <vulkan/vulkan.h>

namespace Raphael
{

class VulkanRenderer : public RendererAPI
{
    virtual void Init() override;
    virtual void Shutdown() override;

    virtual void BeginFrame() override;
    virtual void EndFrame() override;

    virtual const RendererCapabilities &GetCapabilities() const override;
};

namespace Utils
{

    void InsertImageMemoryBarrier(VkCommandBuffer cmdbuffer, VkImage image, VkAccessFlags srcAccessMask,
                                  VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout,
                                  VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask,
                                  VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange);

    void SetImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkImageLayout oldImageLayout,
                        VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange,
                        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    void SetImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkImageAspectFlags aspectMask,
                        VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
                        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

}    // namespace Utils

}    // namespace Raphael

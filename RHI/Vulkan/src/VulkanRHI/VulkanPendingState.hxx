#pragma once

#include "VulkanRHI/VulkanCommandContext.hxx"

#include "VulkanRHI/Resources/VulkanGraphicsPipeline.hxx"
#include "VulkanRHI/VulkanCommandsObjects.hxx"

namespace VulkanRHI
{

class VulkanPendingState : public IDeviceChild
{
public:
    VulkanPendingState(VulkanDevice* InDevice, VulkanCommandContext& InCmdContext);
    ~VulkanPendingState();

    void Reset();

    void SetViewport(glm::vec3 Min, glm::vec3 Max)
    {
        Viewports[0] = {
            .x = Min.x,
            .y = Min.y,
            .width = Max.x,
            .height = Max.y,
            .minDepth = Min.z,
            .maxDepth = Max.z,
        };
    }

    void SetScissor(glm::ivec2 Offset, glm::uvec2 Extent)
    {
        Scissors[0] = {
            .offset = {Offset.x, Offset.y},
            .extent = {Extent.x, Extent.y},
        };
    }

    void Bind(VkCommandBuffer CmdBuffer)
    {
        CurrentPipeline->Bind(CmdBuffer);
    }

    void PrepareForDraw(VulkanCmdBuffer* CommandBuffer);
    bool SetGraphicsPipeline(Ref<VulkanGraphicsPipeline>& InPipeline, bool bForceReset = false);

private:
    Array<VkViewport> Viewports;
    Array<VkRect2D> Scissors;

    Ref<VulkanGraphicsPipeline> CurrentPipeline = nullptr;
    VulkanCommandContext& CmdContext;
};

}    // namespace VulkanRHI

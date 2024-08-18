#pragma once

#include "VulkanRHI/Resources/VulkanBuffer.hxx"
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

    void SetViewport(FVector3 Min, FVector3 Max)
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

    void SetScissor(IVector2 Offset, UVector2 Extent)
    {
        Scissors[0] = {
            .offset = {Offset.x, Offset.y},
            .extent = {Extent.x, Extent.y},
        };
    }

    void SetVertexBuffer(Ref<VulkanBuffer>& Buffer, uint32 BufferIndex = 0, uint32 Offset = 0);

    void Bind(VkCommandBuffer CmdBuffer)
    {
        CurrentPipeline->Bind(CmdBuffer);
    }

    void PrepareForDraw(VulkanCmdBuffer* CommandBuffer);
    bool SetGraphicsPipeline(Ref<VulkanGraphicsPipeline>& InPipeline, bool bForceReset = false);

private:
    Array<VkViewport> Viewports;
    Array<VkRect2D> Scissors;

    struct FVertexSource {
        Ref<VulkanBuffer> Buffer = nullptr;
        uint32 Offset = 0;
    };
    Array<FVertexSource> VertexSources;

    Ref<VulkanGraphicsPipeline> CurrentPipeline = nullptr;
    VulkanCommandContext& CmdContext;
};

}    // namespace VulkanRHI

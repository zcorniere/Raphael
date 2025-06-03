#pragma once

#include "VulkanRHI/Resources/VulkanBuffer.hxx"
#include "VulkanRHI/VulkanCommandContext.hxx"

#include "VulkanRHI/Resources/VulkanGraphicsPipeline.hxx"
#include "VulkanRHI/VulkanCommandsObjects.hxx"

namespace VulkanRHI
{

class FVulkanPendingState : public IDeviceChild
{
public:
    FVulkanPendingState(FVulkanDevice* InDevice, FVulkanCommandContext& InCmdContext);
    ~FVulkanPendingState();

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

    void SetVertexBuffer(Ref<RVulkanBuffer>& Buffer, uint32 BufferIndex = 0, uint32 Offset = 0);
    bool SetGraphicsPipeline(Ref<RVulkanGraphicsPipeline>& InPipeline, bool bForceReset = false);
    bool SetPendingDescriptorSets(TArray<VkDescriptorSet> InDescriptorSet)
    {
        DescriptorSets = InDescriptorSet;
        return true;
    }

    void Bind(VkCommandBuffer CmdBuffer)
    {
        CurrentPipeline->Bind(CmdBuffer);
    }

    template <typename T>
    requires std::is_standard_layout_v<T>
    void SetPushConstant(const T& Data)
    {
        PushConstantData.Resize(sizeof(T));
        std::memcpy(PushConstantData.Raw(), &Data, sizeof(T));
    }

    void PrepareForDraw(FVulkanCmdBuffer* CommandBuffer);

private:
    TArray<uint8> PushConstantData;

    TArray<VkViewport> Viewports;
    TArray<VkRect2D> Scissors;

    struct FVertexSource
    {
        Ref<RVulkanBuffer> Buffer = nullptr;
        uint32 Offset = 0;
    };
    TArray<FVertexSource> VertexSources;

    TArray<VkDescriptorSet> DescriptorSets;
    Ref<RVulkanGraphicsPipeline> CurrentPipeline = nullptr;
    FVulkanCommandContext& CmdContext;
};

}    // namespace VulkanRHI

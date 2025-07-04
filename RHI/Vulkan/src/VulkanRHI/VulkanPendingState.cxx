#include "VulkanRHI/VulkanPendingState.hxx"

namespace VulkanRHI
{

FVulkanPendingState::FVulkanPendingState(FVulkanDevice* InDevice, FVulkanCommandContext& InCmdContext)
    : IDeviceChild(InDevice)
    , CmdContext(InCmdContext)
{
    Reset();
}

FVulkanPendingState::~FVulkanPendingState()
{
}

void FVulkanPendingState::Reset()
{
    Viewports.Clear();
    Viewports.Resize(1);
    Scissors.Clear();
    Scissors.Resize(1);

    CurrentPipeline = nullptr;
    DescriptorSets.Clear();
}

void FVulkanPendingState::SetVertexBuffer(Ref<RVulkanBuffer>& Buffer, uint32 BufferIndex, uint32 Offset)
{
    check(EnumHasAnyFlags(EBufferUsageFlags::VertexBuffer, Buffer->GetUsage()));
    if (VertexSources.Size() <= BufferIndex)
    {
        VertexSources.Resize(BufferIndex + 1);
    }
    VertexSources[BufferIndex] = {Buffer, Offset};
}

bool FVulkanPendingState::SetGraphicsPipeline(Ref<RVulkanGraphicsPipeline>& InPipeline, bool bForceReset)
{
    bool bNeedReset = bForceReset;

    if (CurrentPipeline != InPipeline)
    {
        CurrentPipeline = InPipeline;
        bNeedReset = true;
    }
    DescriptorSets.Clear();

    return bNeedReset;
}

void FVulkanPendingState::PrepareForDraw(FVulkanCmdBuffer* CommandBuffer)
{
    if (Viewports.Size() > 0)
    {
        VulkanAPI::vkCmdSetViewport(CommandBuffer->GetHandle(), 0, Viewports.Size(), Viewports.Raw());
    }

    if (Scissors.Size() > 0)
    {
        VulkanAPI::vkCmdSetScissor(CommandBuffer->GetHandle(), 0, Scissors.Size(), Scissors.Raw());
    }

    CurrentPipeline->Bind(CommandBuffer->GetHandle());
    for (VkDescriptorSet Set: DescriptorSets)
    {
        VulkanAPI::vkCmdBindDescriptorSets(CommandBuffer->GetHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                           CurrentPipeline->GetPipelineLayout(), 0, 1, &Set, 0, nullptr);
    }
    if (PushConstantData.Size() > 0)
    {
        VulkanAPI::vkCmdPushConstants(CommandBuffer->GetHandle(), CurrentPipeline->GetPipelineLayout(),
                                      VK_SHADER_STAGE_VERTEX_BIT, 0, PushConstantData.Size(), PushConstantData.Raw());
    }

    TArray<VkBuffer> VertexBuffers;
    VertexBuffers.Reserve(VertexSources.Size());
    TArray<VkDeviceSize> Offsets;
    Offsets.Reserve(VertexSources.Size());

    for (const FVertexSource& VertexSource: VertexSources)
    {
        VertexBuffers.Add(VertexSource.Buffer->GetHandle());
        Offsets.Add(VertexSource.Offset);
    }
    VulkanAPI::vkCmdBindVertexBuffers(CommandBuffer->GetHandle(), 0, VertexBuffers.Size(), VertexBuffers.Raw(),
                                      Offsets.Raw());
}
}    // namespace VulkanRHI

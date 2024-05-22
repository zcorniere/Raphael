#include "VulkanRHI/VulkanPendingState.hxx"

namespace VulkanRHI
{

VulkanPendingState::VulkanPendingState(VulkanDevice* InDevice, VulkanCommandContext& InCmdContext)
    : IDeviceChild(InDevice), CmdContext(InCmdContext)
{
    Reset();
}

VulkanPendingState::~VulkanPendingState()
{
}

void VulkanPendingState::Reset()
{
    Viewports.Clear();
    Viewports.Resize(1);
    Scissors.Clear();
    Scissors.Resize(1);

    CurrentPipeline = nullptr;
}

bool VulkanPendingState::SetGraphicsPipeline(Ref<VulkanGraphicsPipeline>& InPipeline, bool bForceReset)
{
    bool bNeedReset = bForceReset;

    if (CurrentPipeline != InPipeline) {
        CurrentPipeline = InPipeline;
        bNeedReset = true;
    }
    /// TODO: reset descriptor sets

    return bNeedReset;
}

void VulkanPendingState::PrepareForDraw(VulkanCmdBuffer* CommandBuffer)
{
    if (Viewports.Size() > 0) {
        VulkanAPI::vkCmdSetViewport(CommandBuffer->GetHandle(), 0, Viewports.Size(), Viewports.Raw());
    }

    if (Scissors.Size() > 0) {
        VulkanAPI::vkCmdSetScissor(CommandBuffer->GetHandle(), 0, Scissors.Size(), Scissors.Raw());
    }

    CurrentPipeline->Bind(CommandBuffer->GetHandle());

    // TODO: bind descriptor sets
    // TODO: bind pipeline layout
    // TODO: bind Vertex/index buffers
}

}    // namespace VulkanRHI

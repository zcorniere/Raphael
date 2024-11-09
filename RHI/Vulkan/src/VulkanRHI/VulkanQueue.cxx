#include "VulkanRHI/VulkanQueue.hxx"

#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanSynchronization.hxx"

namespace VulkanRHI
{

FVulkanQueue::FVulkanQueue(FVulkanDevice* InDevice, std::uint32_t InFamilyIndex)
    : IDeviceChild(InDevice), Queue(VK_NULL_HANDLE), FamilyIndex(InFamilyIndex), QueueIndex(0)
{
    VulkanAPI::vkGetDeviceQueue(Device->GetHandle(), FamilyIndex, QueueIndex, &Queue);
}

FVulkanQueue::~FVulkanQueue()
{
}

void FVulkanQueue::Submit(FVulkanCmdBuffer* CmdBuffer, uint32 NumSignaledSemaphores, VkSemaphore* SignalSemaphores)
{
    RPH_PROFILE_FUNC()

    check(CmdBuffer && CmdBuffer->HasEnded());

    Ref<RFence>& Fence = CmdBuffer->m_Fence;
    check(!Fence->IsSignaled());

    const VkCommandBuffer CmdBuffers[] = {
        CmdBuffer->GetHandle(),
    };

    VkSubmitInfo SubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = CmdBuffers,
        .signalSemaphoreCount = NumSignaledSemaphores,
        .pSignalSemaphores = SignalSemaphores,
    };

    TArray<VkSemaphore> WaitSemaphores;
    if (!CmdBuffer->WaitSemaphore.IsEmpty()) {
        WaitSemaphores.Reserve(CmdBuffer->WaitSemaphore.Size());
        for (Ref<RSemaphore>& Semaphore: CmdBuffer->WaitSemaphore) {
            WaitSemaphores.Add(Semaphore->GetHandle());
        }
        SubmitInfo.waitSemaphoreCount = CmdBuffer->WaitSemaphore.Size();
        SubmitInfo.pWaitSemaphores = WaitSemaphores.Raw();
        SubmitInfo.pWaitDstStageMask = CmdBuffer->WaitFlags.Raw();
    }
    VK_CHECK_RESULT(VulkanAPI::vkQueueSubmit(Queue, 1, &SubmitInfo, Fence->GetHandle()));

    CmdBuffer->State = FVulkanCmdBuffer::EState::Submitted;
    CmdBuffer->WaitSemaphore.Clear();
}

void FVulkanQueue::SetName(std::string_view InName)
{
    FNamedClass::SetName(InName);
    VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_QUEUE, Queue, "{:s}", InName);
}

}    // namespace VulkanRHI

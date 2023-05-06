#include "VulkanRHI/VulkanQueue.hxx"

#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanSynchronization.hxx"

namespace VulkanRHI
{

VulkanQueue::VulkanQueue(Ref<VulkanDevice> InDevice, std::uint32_t InFamilyIndex)
    : Queue(VK_NULL_HANDLE), FamilyIndex(InFamilyIndex), QueueIndex(0), Device(InDevice)
{
    VulkanAPI::vkGetDeviceQueue(Device->GetInstanceHandle(), FamilyIndex, QueueIndex, &Queue);
}

VulkanQueue::~VulkanQueue() {}

void VulkanQueue::Submit(Ref<VulkanCmdBuffer> &CmdBuffer, uint32 NumSignaledSemaphores, VkSemaphore *SignalSemaphores)
{
    check(CmdBuffer->HasEnded());

    Ref<Fence> &Fence = CmdBuffer->m_Fence;
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

    std::vector<VkSemaphore> WaitSemaphores;
    if (!CmdBuffer->WaitSemaphore.empty()) {
        WaitSemaphores.reserve(CmdBuffer->WaitSemaphore.size());
        for (Ref<Semaphore> &Semaphore: CmdBuffer->WaitSemaphore) { WaitSemaphores.push_back(Semaphore->GetHandle()); }
        SubmitInfo.waitSemaphoreCount = CmdBuffer->WaitSemaphore.size();
        SubmitInfo.pWaitSemaphores = WaitSemaphores.data();
        SubmitInfo.pWaitDstStageMask = CmdBuffer->WaitFlags.data();
    }
    VK_CHECK_RESULT(VulkanAPI::vkQueueSubmit(Queue, 1, &SubmitInfo, Fence->GetHandle()));

    CmdBuffer->State = VulkanCmdBuffer::EState::Submitted;
    CmdBuffer->WaitSemaphore.clear();
}

}    // namespace VulkanRHI

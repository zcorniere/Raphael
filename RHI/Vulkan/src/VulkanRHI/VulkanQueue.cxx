#include "VulkanRHI/VulkanQueue.hxx"

#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanSynchronization.hxx"

namespace VulkanRHI
{

VulkanQueue::VulkanQueue(VulkanDevice* InDevice, std::uint32_t InFamilyIndex)
    : Queue(VK_NULL_HANDLE), FamilyIndex(InFamilyIndex), QueueIndex(0), Device(InDevice)
{
    VulkanAPI::vkGetDeviceQueue(Device->GetHandle(), FamilyIndex, QueueIndex, &Queue);
}

VulkanQueue::~VulkanQueue()
{
}

void VulkanQueue::SetName(std::string_view InName)
{
    NamedClassWithTypeName::SetName(InName);
    VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_QUEUE, Queue, "[Queue] {:s}", InName);
}

void VulkanQueue::Submit(VulkanCmdBuffer* CmdBuffer, uint32 NumSignaledSemaphores, VkSemaphore* SignalSemaphores)
{
    RPH_PROFILE_FUNC()

    check(CmdBuffer->HasEnded());

    Fence* Fence = CmdBuffer->m_Fence;
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

    Array<VkSemaphore> WaitSemaphores;
    if (!CmdBuffer->WaitSemaphore.IsEmpty()) {
        WaitSemaphores.Reserve(CmdBuffer->WaitSemaphore.Size());
        for (Semaphore* Semaphore: CmdBuffer->WaitSemaphore) {
            WaitSemaphores.Add(Semaphore->GetHandle());
        }
        SubmitInfo.waitSemaphoreCount = CmdBuffer->WaitSemaphore.Size();
        SubmitInfo.pWaitSemaphores = WaitSemaphores.Raw();
        SubmitInfo.pWaitDstStageMask = CmdBuffer->WaitFlags.Raw();
    }
    VK_CHECK_RESULT(VulkanAPI::vkQueueSubmit(Queue, 1, &SubmitInfo, Fence->GetHandle()));

    CmdBuffer->State = VulkanCmdBuffer::EState::Submitted;
    CmdBuffer->WaitSemaphore.Clear();
}

}    // namespace VulkanRHI

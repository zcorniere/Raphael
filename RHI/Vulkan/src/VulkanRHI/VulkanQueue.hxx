#pragma once

#include "VulkanRHI/VulkanUtils.hxx"

namespace VulkanRHI
{

class VulkanCmdBuffer;
class VulkanDevice;

class VulkanQueue : public RObject
{
public:
    VulkanQueue(Ref<VulkanDevice> InDevice, std::uint32_t InFamilyIndex);
    ~VulkanQueue();

    inline std::uint32_t GetFamilyIndex() const { return FamilyIndex; }

    inline std::uint32_t GetQueueIndex() const { return QueueIndex; }

    inline VkQueue GetHandle() const { return Queue; }

    void Submit(Ref<VulkanCmdBuffer> &CmdBuffer, uint32 NumSignaledSemaphores = 0,
                VkSemaphore *SignalSemaphores = nullptr);

    void Submit(Ref<VulkanCmdBuffer> &CmdBuffer, VkSemaphore SignalSemaphores)
    {
        return Submit(CmdBuffer, 1, &SignalSemaphores);
    }

private:
    VkQueue Queue;
    std::uint32_t FamilyIndex;
    std::uint32_t QueueIndex;
    Ref<VulkanDevice> Device;
};

}    // namespace VulkanRHI

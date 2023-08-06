#pragma once

#include "VulkanRHI/VulkanUtils.hxx"

namespace VulkanRHI
{

class VulkanCmdBuffer;
class VulkanDevice;

class VulkanQueue : public NamedClass
{
public:
    VulkanQueue(VulkanDevice* InDevice, std::uint32_t InFamilyIndex);
    ~VulkanQueue();

    inline std::uint32_t GetFamilyIndex() const
    {
        return FamilyIndex;
    }

    inline std::uint32_t GetQueueIndex() const
    {
        return QueueIndex;
    }

    inline VkQueue GetHandle() const
    {
        return Queue;
    }

    void Submit(VulkanCmdBuffer* CmdBuffer, uint32 NumSignaledSemaphores = 0, VkSemaphore* SignalSemaphores = nullptr);

    void Submit(VulkanCmdBuffer* CmdBuffer, VkSemaphore SignalSemaphores)
    {
        return Submit(CmdBuffer, 1, &SignalSemaphores);
    }

    void SetName(std::string_view InName) override;

private:
    VulkanDevice* Device;

    VkQueue Queue;
    std::uint32_t FamilyIndex;
    std::uint32_t QueueIndex;
};

}    // namespace VulkanRHI

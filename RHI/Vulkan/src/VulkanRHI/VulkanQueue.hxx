#pragma once

#include "VulkanRHI/VulkanUtils.hxx"

namespace VulkanRHI
{

class VulkanCmdBuffer;
class VulkanDevice;

class VulkanQueue final : public NamedClassWithTypeName<VulkanQueue>
{
public:
    VulkanQueue(VulkanDevice* InDevice, std::uint32_t InFamilyIndex);
    ~VulkanQueue();

    void SetName(std::string_view InName) override;

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

private:
    VkQueue Queue;
    std::uint32_t FamilyIndex;
    std::uint32_t QueueIndex;
    VulkanDevice* Device;
};

}    // namespace VulkanRHI

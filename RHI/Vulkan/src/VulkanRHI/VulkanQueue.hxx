#pragma once

namespace VulkanRHI
{

class FVulkanCmdBuffer;
class FVulkanDevice;

class FVulkanQueue : public FNamedClass, public IDeviceChild
{
public:
    FVulkanQueue(FVulkanDevice* InDevice, std::uint32_t InFamilyIndex);
    virtual ~FVulkanQueue();

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

    void Submit(FVulkanCmdBuffer* CmdBuffer, uint32 NumSignaledSemaphores = 0, VkSemaphore* SignalSemaphores = nullptr);

    void Submit(FVulkanCmdBuffer* CmdBuffer, VkSemaphore SignalSemaphores)
    {
        return Submit(CmdBuffer, 1, &SignalSemaphores);
    }

    void SetName(std::string_view InName) override;

private:
    VkQueue Queue;
    std::uint32_t FamilyIndex;
    std::uint32_t QueueIndex;
};

}    // namespace VulkanRHI

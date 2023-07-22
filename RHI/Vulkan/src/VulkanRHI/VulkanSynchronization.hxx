#pragma once

#include "VulkanRHI/VulkanLoader.hxx"

namespace VulkanRHI
{

class VulkanDevice;

void VulkanSetImageLayout(VkCommandBuffer CmdBuffer, VkImage Image, VkImageLayout OldLayout, VkImageLayout NewLayout,
                          const VkImageSubresourceRange& SubresourceRange);

class Barrier
{
public:
    static VkImageSubresourceRange MakeSubresourceRange(VkImageAspectFlags AspectMask, uint32 FirstMip = 0,
                                                        uint32 NumMips = VK_REMAINING_MIP_LEVELS, uint32 FirstLayer = 0,
                                                        uint32 NumLayers = VK_REMAINING_ARRAY_LAYERS);

public:
    Barrier();

    void TransitionLayout(VkImage Image, VkImageLayout OldLayout, VkImageLayout NewLayout,
                          const VkImageSubresourceRange& SubresourceRange);
    void Execute(VkCommandBuffer CmdBuffer);

private:
    std::vector<VkImageMemoryBarrier> ImageBarrier;
};

class Semaphore : public RObject
{
public:
    Semaphore(VulkanDevice* InDevice);
    virtual ~Semaphore();

    virtual void SetName(std::string_view InName) override final;

    inline VkSemaphore GetHandle() const
    {
        return SemaphoreHandle;
    }

private:
    VulkanDevice* Device;
    VkSemaphore SemaphoreHandle;
};

class Fence : public RObject
{
protected:
    enum class State {
        // Initial state
        NotReady,

        // After GPU processed it
        Signaled,
    };

public:
    Fence(VulkanDevice* InDevice, bool bCreateSignaled);
    ~Fence();

    virtual void SetName(std::string_view InName) override final;

    inline VkFence GetHandle() const
    {
        return Handle;
    }

    void Reset();
    bool Wait(uint64 TimeInNanoseconds);

    inline bool IsSignaled()
    {
        return State == State::Signaled || CheckFenceStatus();
    }

private:
    bool CheckFenceStatus();

private:
    VulkanDevice* Device;
    VkFence Handle;
    State State;
};

}    // namespace VulkanRHI

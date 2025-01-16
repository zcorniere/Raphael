#pragma once

#include "VulkanRHI/VulkanLoader.hxx"

namespace VulkanRHI
{

class FVulkanDevice;

void VulkanSetImageLayout(VkCommandBuffer CmdBuffer, VkImage Image, VkImageLayout OldLayout, VkImageLayout NewLayout,
                          const VkImageSubresourceRange& SubresourceRange);

class FBarrier
{
public:
    static VkImageSubresourceRange MakeSubresourceRange(VkImageAspectFlags AspectMask, uint32 FirstMip = 0,
                                                        uint32 NumMips = VK_REMAINING_MIP_LEVELS, uint32 FirstLayer = 0,
                                                        uint32 NumLayers = VK_REMAINING_ARRAY_LAYERS);

public:
    FBarrier();

    void TransitionLayout(VkImage Image, VkImageLayout OldLayout, VkImageLayout NewLayout,
                          const VkImageSubresourceRange& SubresourceRange);
    void Execute(VkCommandBuffer CmdBuffer);

private:
    TArray<VkImageMemoryBarrier> ImageBarrier;
};

class RSemaphore : public RObject, public IDeviceChild
{
    RTTI_DECLARE_TYPEINFO(RSemaphore, RObject);

public:
    RSemaphore(FVulkanDevice* InDevice);
    virtual ~RSemaphore();

    virtual void SetName(std::string_view InName) override final;

    inline VkSemaphore GetHandle() const
    {
        return SemaphoreHandle;
    }

private:
    VkSemaphore SemaphoreHandle;
};

class RFence : public RObject, public IDeviceChild
{
    RTTI_DECLARE_TYPEINFO(RFence, RObject);

protected:
    enum class EState {
        // Initial state
        NotReady,

        // After GPU processed it
        Signaled,
    };

public:
    RFence(FVulkanDevice* InDevice, bool bCreateSignaled);
    ~RFence();

    virtual void SetName(std::string_view InName) override final;

    inline VkFence GetHandle() const
    {
        return Handle;
    }

    void Reset();
    bool Wait(uint64 TimeInNanoseconds);

    inline bool IsSignaled()
    {
        return State == EState::Signaled || CheckFenceStatus();
    }

private:
    bool CheckFenceStatus();

private:
    VkFence Handle;
    EState State;
};

}    // namespace VulkanRHI

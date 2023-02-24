#pragma once

#include "Engine/Renderer/Vulkan/VulkanLoader.hxx"

namespace VulkanRHI
{

class VulkanDevice;

class Semaphore : public RObject
{
public:
    Semaphore(Ref<VulkanDevice> &InDevice);
    virtual ~Semaphore();

    inline VkSemaphore GetHandle() const
    {
        return SemaphoreHandle;
    }

private:
    Ref<VulkanDevice> Device;
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
    Fence(Ref<VulkanDevice> InDevice, bool bCreateSignaled);
    ~Fence();

    inline VkFence GetHandle() const
    {
        return Handle;
    }

    void Reset();
    bool Wait(uint64 TimeInNanoseconds);

    inline bool IsSignaled() const
    {
        return State == State::Signaled;
    }

private:
    Ref<VulkanDevice> Device;
    VkFence Handle;
    State State;
};

}    // namespace VulkanRHI

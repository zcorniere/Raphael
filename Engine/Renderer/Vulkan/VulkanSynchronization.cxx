#include "Engine/Renderer/Vulkan/VulkanSynchronization.hxx"
#include "Engine/Renderer/Vulkan/VulkanDevice.hxx"

#include "Engine/Renderer/Vulkan/VulkanUtils.hxx"

namespace VulkanRHI
{

Semaphore::Semaphore(Ref<VulkanDevice> &InDevice): Device(InDevice), SemaphoreHandle(VK_NULL_HANDLE)
{
    VkSemaphoreCreateInfo CreateInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    VK_CHECK_RESULT(VulkanAPI::vkCreateSemaphore(Device->GetInstanceHandle(), &CreateInfo, nullptr, &SemaphoreHandle));
}

Semaphore::~Semaphore()
{
    if (SemaphoreHandle) { VulkanAPI::vkDestroySemaphore(Device->GetInstanceHandle(), SemaphoreHandle, nullptr); }
    SemaphoreHandle = VK_NULL_HANDLE;
}

Fence::Fence(Ref<VulkanDevice> InDevice, bool bCreateSignaled)
    : Device(InDevice), State(bCreateSignaled ? Fence::State::Signaled : Fence::State::NotReady)
{
    VkFenceCreateInfo Info{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = 0,
    };
    if (bCreateSignaled) { Info.flags = VK_FENCE_CREATE_SIGNALED_BIT; }
    VK_CHECK_RESULT(VulkanAPI::vkCreateFence(Device->GetInstanceHandle(), &Info, nullptr, &Handle));
}

Fence::~Fence()
{
    VulkanAPI::vkDestroyFence(Device->GetInstanceHandle(), Handle, nullptr);
}

void Fence::Reset()
{
    if (State != State::NotReady) {
        VK_CHECK_RESULT(VulkanAPI::vkResetFences(Device->GetInstanceHandle(), 1, &Handle));
        State = State::NotReady;
    }
}

bool Fence::Wait(uint64 TimeInNanoseconds)
{
    check(State == State::NotReady);

    VkResult Result = VulkanAPI::vkWaitForFences(Device->GetInstanceHandle(), 1, &Handle, true, TimeInNanoseconds);
    switch (Result) {
        case VK_SUCCESS: State = State::Signaled; return true;
        case VK_TIMEOUT: break;
        default: VK_CHECK_RESULT_EXPANDED(Result); break;
    }
    return false;
}

bool Fence::CheckFenceStatus()
{
    check(State == State::NotReady);
    VkResult Result = VulkanAPI::vkGetFenceStatus(Device->GetInstanceHandle(), Handle);
    switch (Result) {
        case VK_SUCCESS: State = State::Signaled; return true;

        case VK_NOT_READY: break;

        default: VK_CHECK_RESULT(Result); break;
    }

    return false;
}

}    // namespace VulkanRHI

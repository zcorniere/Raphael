#include "Engine/Renderer/Vulkan/VulkanQueue.hxx"

#include "Engine/Renderer/Vulkan/VulkanDevice.hxx"

namespace VulkanRHI
{

VulkanQueue::VulkanQueue(Ref<VulkanDevice> InDevice, std::uint32_t InFamilyIndex)
    : Queue(VK_NULL_HANDLE), FamilyIndex(InFamilyIndex), QueueIndex(0), Device(InDevice)
{
    VulkanAPI::vkGetDeviceQueue(Device->GetInstanceHandle(), FamilyIndex, QueueIndex, &Queue);
}

VulkanQueue::~VulkanQueue()
{
}

}    // namespace VulkanRHI

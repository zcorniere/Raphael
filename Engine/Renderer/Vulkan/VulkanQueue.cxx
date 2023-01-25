#include "Engine/Renderer/Vulkan/VulkanQueue.hxx"

#include "Engine/Renderer/Vulkan/VulkanDevice.hxx"

namespace Raphael::RHI
{

VulkanQueue::VulkanQueue(VulkanDevice *InDevice, std::uint32_t InFamilyIndex)
    : Queue(VK_NULL_HANDLE), FamilyIndex(InFamilyIndex), QueueIndex(0), Device(InDevice)
{
    VulkanAPI::vkGetDeviceQueue(Device->GetInstanceHandle(), FamilyIndex, QueueIndex, &Queue);
}

VulkanQueue::~VulkanQueue()
{
    check(Device);
}

}    // namespace Raphael::RHI

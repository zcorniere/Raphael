#include "Engine/Renderer/Vulkan/VulkanGenericPlatform.hxx"

namespace Raphael::RHI
{

VkResult VulkanGenericPlatform::Present(VkQueue Queue, VkPresentInfoKHR &PresentInfo)
{
    // return VulkanRHI::vkQueuePresentKHR(Queue, &PresentInfo);
}

VkResult VulkanGenericPlatform::CreateSwapchainKHR(VkDevice Device, const VkSwapchainCreateInfoKHR *CreateInfo,
                                                   const VkAllocationCallbacks *Allocator, VkSwapchainKHR *Swapchain)
{
    // return VulkanRHI::vkCreateSwapchainKHR(Device, CreateInfo, Allocator, Swapchain);
}

void VulkanGenericPlatform::DestroySwapchainKHR(VkDevice Device, VkSwapchainKHR Swapchain,
                                                const VkAllocationCallbacks *Allocator)
{
    // VulkanRHI::vkDestroySwapchainKHR(Device, Swapchain, Allocator);
}
}    // namespace Raphael::RHI

#include "Engine/Renderer/Vulkan/VulkanGenericPlatform.hxx"

#include "Engine/Renderer/Vulkan/VulkanLoader.hxx"

namespace Raphael::RHI
{

VkResult VulkanGenericPlatform::Present(VkQueue Queue, VkPresentInfoKHR &PresentInfo)
{
    return VulkanAPI::vkQueuePresentKHR(Queue, &PresentInfo);
}

VkResult VulkanGenericPlatform::CreateSwapchainKHR(VkDevice Device, const VkSwapchainCreateInfoKHR *CreateInfo,
                                                   const VkAllocationCallbacks *Allocator, VkSwapchainKHR *Swapchain)
{
    return VulkanAPI::vkCreateSwapchainKHR(Device, CreateInfo, Allocator, Swapchain);
}

void VulkanGenericPlatform::DestroySwapchainKHR(VkDevice Device, VkSwapchainKHR Swapchain,
                                                const VkAllocationCallbacks *Allocator)
{
    VulkanAPI::vkDestroySwapchainKHR(Device, Swapchain, Allocator);
}
}    // namespace Raphael::RHI

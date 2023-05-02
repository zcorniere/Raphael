#include "VulkanRHI/Resources/VulkanViewport.hxx"

#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanRHI.hxx"
#include "VulkanRHI/VulkanSwapChain.hxx"
#include "VulkanRHI/VulkanSynchronization.hxx"

namespace VulkanRHI
{

VulkanViewport::VulkanViewport(Ref<VulkanDevice> InDevice, void *InWindowHandle, glm::uvec2 InSize)
    : Device(InDevice), WindowHandle(InWindowHandle), Size(InSize)
{
    CreateSwapchain();
}

VulkanViewport::~VulkanViewport()
{
    for (VulkanTextureView &View: TexturesViews) { View.Destroy(Device); }
    TexturesViews.clear();
    RenderingDoneSemaphores.clear();

    SwapChain->Destroy(nullptr);
    SwapChain = nullptr;
}

void VulkanViewport::SetName(std::string_view InName)
{
    RHIResource::SetName(InName);

    if (!SwapChain) return;

    VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_SWAPCHAIN_KHR, SwapChain->GetHandle(), "Swapchain - \"{:s}\"", InName);

    check(BackBufferImages.size() == RenderingDoneSemaphores.size());
    check(BackBufferImages.size() == TexturesViews.size());

    for (unsigned i = 0; i < BackBufferImages.size(); i++) {
        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_SEMAPHORE, RenderingDoneSemaphores.at(i)->GetHandle(),
                              "Swapchain \"{:s}\" - Semaphore Rendering done({})", InName, i);
        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_IMAGE, TexturesViews.at(i).Image,
                              "Swapchain \"{:s}\" - Image ({})", InName, i);
        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_IMAGE_VIEW, TexturesViews.at(i).View,
                              "Swapchain \"{:s}\" - Image View ({})", InName, i);
    }
}

void VulkanViewport::CreateSwapchain()
{
    Ref<VulkanDynamicRHI> RHI = GetVulkanDynamicRHI();

    SwapChain = Ref<VulkanSwapChain>::Create(RHI->RHIGetVkInstance(), Device, WindowHandle, Size, 0, BackBufferImages,
                                             true, nullptr);

    RenderingDoneSemaphores.resize(BackBufferImages.size());
    for (unsigned i = 0; i < BackBufferImages.size(); i++) {
        RenderingDoneSemaphores[i] = Ref<Semaphore>::Create(Device);
    }

    TexturesViews.resize(BackBufferImages.size());
    for (unsigned i = 0; i < BackBufferImages.size(); i++) {
        TexturesViews.at(i).Create(Device, BackBufferImages.at(i), VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT,
                                   SwapChain->GetFormat(), 0, 1);
    }

    // Refresh vulkan object name
    VulkanViewport::SetName(GetName());
}

}    // namespace VulkanRHI

#include "Engine/Renderer/Vulkan/VulkanResources.hxx"

#include "Engine/Renderer/Vulkan/IVulkanDynamicRHI.hxx"
#include "Engine/Renderer/Vulkan/VulkanDevice.hxx"
#include "Engine/Renderer/Vulkan/VulkanSynchronization.hxx"

VulkanViewport::VulkanViewport(Ref<VulkanDevice> InDevice, void *InWindowHandle, glm::uvec2 InSize)
    : Device(InDevice), WindowHandle(InWindowHandle), Size(InSize)
{
    CreateSwapchain();
}

VulkanViewport::~VulkanViewport()
{
    SwapChain->Destroy(nullptr);
}

void VulkanViewport::CreateSwapchain()
{
    Ref<IVulkanDynamicRHI> RHI = GetIVulkanDynamicRHI();

    SwapChain = Ref<VulkanSwapChain>::Create(RHI->RHIGetVkInstance(), Device, WindowHandle, Size, 0, BackBufferImages,
                                             false, nullptr);

    RenderingDoneSemaphores.resize(BackBufferImages.size());
    for (Ref<Semaphore> &SemaphoreRef: RenderingDoneSemaphores) {
        SemaphoreRef = Ref<Semaphore>::Create(Device);
    }
}

#include "Engine/Renderer/Vulkan/VulkanSwapchain.hxx"

#include "Engine/Renderer/Renderer.hxx"

#include <GLFW/glfw3.h>

// Macro to get a procedure address based on a vulkan instance
#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)                                                              \
    {                                                                                                         \
        fp##entrypoint = reinterpret_cast<PFN_vk##entrypoint>(vkGetInstanceProcAddr(inst, "vk" #entrypoint)); \
        check(fp##entrypoint);                                                                                \
    }

// Macro to get a procedure address based on a vulkan device
#define GET_DEVICE_PROC_ADDR(dev, entrypoint)                                                              \
    {                                                                                                      \
        fp##entrypoint = reinterpret_cast<PFN_vk##entrypoint>(vkGetDeviceProcAddr(dev, "vk" #entrypoint)); \
        check(fp##entrypoint);                                                                             \
    }

static PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
static PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
static PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
static PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
static PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
static PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
static PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
static PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
static PFN_vkQueuePresentKHR fpQueuePresentKHR;

// Nvidia extensions
PFN_vkCmdSetCheckpointNV fpCmdSetCheckpointNV;
PFN_vkGetQueueCheckpointDataNV fpGetQueueCheckpointDataNV;

VKAPI_ATTR void VKAPI_CALL vkCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void *pCheckpointMarker)
{
    fpCmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
}

VKAPI_ATTR void VKAPI_CALL vkGetQueueCheckpointDataNV(VkQueue queue, uint32_t *pCheckpointDataCount,
                                                      VkCheckpointDataNV *pCheckpointData)
{
    fpGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
}

namespace Raphael
{

DECLARE_LOGGER_CATEGORY(Core, LogVulkanSwapchain, Info);

void VulkanSwapChain::Init(VkInstance instance, const Ref<VulkanDevice> &device)
{
    m_Instance = instance;
    m_Device = device;

    VkDevice vulkanDevice = m_Device->GetVulkanDevice();
    GET_DEVICE_PROC_ADDR(vulkanDevice, CreateSwapchainKHR);
    GET_DEVICE_PROC_ADDR(vulkanDevice, DestroySwapchainKHR);
    GET_DEVICE_PROC_ADDR(vulkanDevice, GetSwapchainImagesKHR);
    GET_DEVICE_PROC_ADDR(vulkanDevice, AcquireNextImageKHR);
    GET_DEVICE_PROC_ADDR(vulkanDevice, QueuePresentKHR);

    GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceSupportKHR);
    GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
    GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceFormatsKHR);
    GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfacePresentModesKHR);

    GET_INSTANCE_PROC_ADDR(instance, CmdSetCheckpointNV);
    GET_INSTANCE_PROC_ADDR(instance, GetQueueCheckpointDataNV);
}

void VulkanSwapChain::InitSurface(GLFWwindow *windowHandle)
{
    VkPhysicalDevice physicalDevice = m_Device->GetPhysicalDevice()->GetVulkanPhysicalDevice();

    glfwCreateWindowSurface(m_Instance, windowHandle, nullptr, &m_Surface);

    // Get available queue family properties
    uint32_t queueCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, NULL);
    check(queueCount >= 1);

    std::vector<VkQueueFamilyProperties> queueProps(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueProps.data());

    // Iterate over each queue to learn whether it supports presenting:
    // Find a queue with present support
    // Will be used to present the swap chain images to the windowing system
    std::vector<VkBool32> supportsPresent(queueCount);
    for (uint32_t i = 0; i < queueCount; i++) {
        fpGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_Surface, &supportsPresent[i]);
    }

    // Search for a graphics and a present queue in the array of queue
    // families, try to find one that supports both
    uint32_t graphicsQueueNodeIndex = UINT32_MAX;
    uint32_t presentQueueNodeIndex = UINT32_MAX;
    for (uint32_t i = 0; i < queueCount; i++) {
        if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            if (graphicsQueueNodeIndex == UINT32_MAX) { graphicsQueueNodeIndex = i; }

            if (supportsPresent[i] == VK_TRUE) {
                graphicsQueueNodeIndex = i;
                presentQueueNodeIndex = i;
                break;
            }
        }
    }
    if (presentQueueNodeIndex == UINT32_MAX) {
        // If there's no queue that supports both present and graphics
        // try to find a separate present queue
        for (uint32_t i = 0; i < queueCount; ++i) {
            if (supportsPresent[i] == VK_TRUE) {
                presentQueueNodeIndex = i;
                break;
            }
        }
    }

    check(graphicsQueueNodeIndex != UINT32_MAX);
    check(presentQueueNodeIndex != UINT32_MAX);

    m_QueueNodeIndex = graphicsQueueNodeIndex;

    FindImageFormatAndColorSpace();
}

void VulkanSwapChain::Create(uint32_t *width, uint32_t *height, bool vsync)
{
    m_VSync = vsync;

    VkDevice device = m_Device->GetVulkanDevice();
    VkPhysicalDevice physicalDevice = m_Device->GetPhysicalDevice()->GetVulkanPhysicalDevice();

    VkSwapchainKHR oldSwapchain = m_SwapChain;

    // Get physical device surface properties and formats
    VkSurfaceCapabilitiesKHR surfCaps;
    VK_CHECK_RESULT(fpGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &surfCaps));

    // Get available present modes
    uint32_t presentModeCount;
    VK_CHECK_RESULT(fpGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, NULL));
    check(presentModeCount > 0);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    VK_CHECK_RESULT(
        fpGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, presentModes.data()));

    VkExtent2D swapchainExtent = {};
    // If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
    if (surfCaps.currentExtent.width == (uint32_t)-1) {
        // If the surface size is undefined, the size is set to
        // the size of the images requested.
        swapchainExtent.width = *width;
        swapchainExtent.height = *height;
    } else {
        // If the surface size is defined, the swap chain size must match
        swapchainExtent = surfCaps.currentExtent;
        *width = surfCaps.currentExtent.width;
        *height = surfCaps.currentExtent.height;
    }

    m_Width = *width;
    m_Height = *height;

    // Select a present mode for the swapchain

    // The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
    // This mode waits for the vertical blank ("v-sync")
    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

    // If v-sync is not requested, try to find a mailbox mode
    // It's the lowest latency non-tearing present mode available
    if (!vsync) {
        for (size_t i = 0; i < presentModeCount; i++) {
            if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
            if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) &&
                (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)) {
                swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
        }
    }

    // Determine the number of images
    uint32_t desiredNumberOfSwapchainImages = surfCaps.minImageCount + 1;
    if ((surfCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfCaps.maxImageCount)) {
        desiredNumberOfSwapchainImages = surfCaps.maxImageCount;
    }

    // Find the transformation of the surface
    VkSurfaceTransformFlagsKHR preTransform;
    if (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        // We prefer a non-rotated transform
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        preTransform = surfCaps.currentTransform;
    }

    // Find a supported composite alpha format (not all devices support alpha opaque)
    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    // Simply select the first composite alpha format available
    std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    for (auto &compositeAlphaFlag: compositeAlphaFlags) {
        if (surfCaps.supportedCompositeAlpha & compositeAlphaFlag) {
            compositeAlpha = compositeAlphaFlag;
            break;
        };
    }

    VkSwapchainCreateInfoKHR swapchainCI{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = NULL,
        .surface = m_Surface,
        .minImageCount = desiredNumberOfSwapchainImages,
        .imageFormat = m_ColorFormat,
        .imageColorSpace = m_ColorSpace,
        .imageExtent = {swapchainExtent.width, swapchainExtent.height},
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL,
        .preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform,
        .compositeAlpha = compositeAlpha,
        .presentMode = swapchainPresentMode,
        // Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
        .clipped = VK_TRUE,
        .oldSwapchain = oldSwapchain,
    };

    // Enable transfer source on swap chain images if supported
    if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    // Enable transfer destination on swap chain images if supported
    if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    VK_CHECK_RESULT(fpCreateSwapchainKHR(device, &swapchainCI, nullptr, &m_SwapChain));

    if (oldSwapchain) fpDestroySwapchainKHR(device, oldSwapchain, nullptr);

    for (auto &image: m_Images)
        vkDestroyImageView(device, image.ImageView, nullptr);
    m_Images.clear();

    VK_CHECK_RESULT(fpGetSwapchainImagesKHR(device, m_SwapChain, &m_ImageCount, NULL));
    // Get the swap chain images
    m_Images.resize(m_ImageCount);
    m_VulkanImages.resize(m_ImageCount);
    VK_CHECK_RESULT(fpGetSwapchainImagesKHR(device, m_SwapChain, &m_ImageCount, m_VulkanImages.data()));

    // Get the swap chain buffers containing the image and imageview
    m_Images.resize(m_ImageCount);
    for (uint32_t i = 0; i < m_ImageCount; i++) {
        VkImageViewCreateInfo colorAttachmentView{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .image = m_VulkanImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_ColorFormat,
            .components =
                {
                    VK_COMPONENT_SWIZZLE_R,
                    VK_COMPONENT_SWIZZLE_G,
                    VK_COMPONENT_SWIZZLE_B,
                    VK_COMPONENT_SWIZZLE_A,
                },
            .subresourceRange =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
        };

        m_Images[i].Image = m_VulkanImages[i];

        VK_CHECK_RESULT(vkCreateImageView(device, &colorAttachmentView, nullptr, &m_Images[i].ImageView));
        VKUtils::SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_IMAGE_VIEW, fmt::format("Swapchain ImageView: {}", i),
                                         m_Images[i].ImageView);
    }

    // Create command buffers
    {
        for (auto &commandBuffer: m_CommandBuffers)
            vkDestroyCommandPool(device, commandBuffer.CommandPool, nullptr);

        VkCommandPoolCreateInfo cmdPoolInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            .queueFamilyIndex = m_QueueNodeIndex,
        };

        VkCommandBufferAllocateInfo commandBufferAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        m_CommandBuffers.resize(m_ImageCount);
        for (auto &commandBuffer: m_CommandBuffers) {
            VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &commandBuffer.CommandPool));

            commandBufferAllocateInfo.commandPool = commandBuffer.CommandPool;
            VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer.CommandBuffer));
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Synchronization Objects
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (!m_Semaphores.RenderComplete || !m_Semaphores.PresentComplete) {
        VkSemaphoreCreateInfo semaphoreCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        VK_CHECK_RESULT(vkCreateSemaphore(m_Device->GetVulkanDevice(), &semaphoreCreateInfo, nullptr,
                                          &m_Semaphores.RenderComplete));
        VKUtils::SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_SEMAPHORE, "Swapchain Semaphore RenderComplete",
                                         m_Semaphores.RenderComplete);
        VK_CHECK_RESULT(vkCreateSemaphore(m_Device->GetVulkanDevice(), &semaphoreCreateInfo, nullptr,
                                          &m_Semaphores.PresentComplete));
        VKUtils::SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_SEMAPHORE, "Swapchain Semaphore PresentComplete",
                                         m_Semaphores.PresentComplete);
    }

    if (m_WaitFences.size() != m_ImageCount) {
        VkFenceCreateInfo fenceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };

        m_WaitFences.resize(m_ImageCount);
        for (auto &fence: m_WaitFences) {
            VK_CHECK_RESULT(vkCreateFence(m_Device->GetVulkanDevice(), &fenceCreateInfo, nullptr, &fence));
            VKUtils::SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_FENCE, "Swapchain Fence", fence);
        }
    }

    VkPipelineStageFlags pipelineStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    m_SubmitInfo = VkSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_Semaphores.PresentComplete,
        .pWaitDstStageMask = &pipelineStageFlags,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m_Semaphores.RenderComplete,
    };

    VkFormat depthFormat = m_Device->GetPhysicalDevice()->GetDepthFormat();

    // Render Pass
    VkAttachmentDescription colorAttachmentDesc{
        // Color attachment
        .format = m_ColorFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference colorReference{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference depthReference{
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpassDescription{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorReference,
        .pResolveAttachments = nullptr,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr,
    };

    VkSubpassDependency dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };

    VkRenderPassCreateInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachmentDesc,
        .subpassCount = 1,
        .pSubpasses = &subpassDescription,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    VK_CHECK_RESULT(vkCreateRenderPass(m_Device->GetVulkanDevice(), &renderPassInfo, nullptr, &m_RenderPass));
    VKUtils::SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_RENDER_PASS, "Swapchain render pass", m_RenderPass);

    // Create framebuffers for every swapchain image
    {
        for (auto &framebuffer: m_Framebuffers)
            vkDestroyFramebuffer(device, framebuffer, nullptr);

        VkFramebufferCreateInfo frameBufferCreateInfo{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = m_RenderPass,
            .attachmentCount = 1,
            .width = m_Width,
            .height = m_Height,
            .layers = 1,
        };

        m_Framebuffers.resize(m_ImageCount);
        for (uint32_t i = 0; i < m_Framebuffers.size(); i++) {
            frameBufferCreateInfo.pAttachments = &m_Images[i].ImageView;
            VK_CHECK_RESULT(
                vkCreateFramebuffer(m_Device->GetVulkanDevice(), &frameBufferCreateInfo, nullptr, &m_Framebuffers[i]));
            VKUtils::SetDebugUtilsObjectName(m_Device->GetVulkanDevice(), VK_OBJECT_TYPE_FRAMEBUFFER,
                                             fmt::format("Swapchain framebuffer (Frame in flight: {})", i),
                                             m_Framebuffers[i]);
        }
    }
}    // namespace Raphael

void VulkanSwapChain::Destroy()
{
    LOG(LogVulkanSwapchain, Info, "Renderer: VulkanSwapChain::OnDestroy");

    auto device = m_Device->GetVulkanDevice();
    vkDeviceWaitIdle(device);

    if (m_SwapChain) fpDestroySwapchainKHR(device, m_SwapChain, nullptr);

    for (auto &image: m_Images)
        vkDestroyImageView(device, image.ImageView, nullptr);

    for (auto &commandBuffer: m_CommandBuffers)
        vkDestroyCommandPool(device, commandBuffer.CommandPool, nullptr);

    if (m_RenderPass) vkDestroyRenderPass(device, m_RenderPass, nullptr);

    for (auto framebuffer: m_Framebuffers)
        vkDestroyFramebuffer(device, framebuffer, nullptr);

    if (m_Semaphores.RenderComplete) vkDestroySemaphore(device, m_Semaphores.RenderComplete, nullptr);

    if (m_Semaphores.PresentComplete) vkDestroySemaphore(device, m_Semaphores.PresentComplete, nullptr);

    for (auto &fence: m_WaitFences)
        vkDestroyFence(device, fence, nullptr);

    vkDeviceWaitIdle(device);
}

void VulkanSwapChain::OnResize(uint32_t width, uint32_t height)
{
    LOG(LogVulkanSwapchain, Info, "Renderer: VulkanSwapChain::OnResize");

    auto device = m_Device->GetVulkanDevice();
    vkDeviceWaitIdle(device);
    Create(&width, &height, m_VSync);
    vkDeviceWaitIdle(device);
}

void VulkanSwapChain::BeginFrame()
{
    // Resource release queue
    // auto &queue = Renderer::GetRenderResourceReleaseQueue(m_CurrentBufferIndex);
    // queue.Execute();

    m_CurrentImageIndex = AcquireNextImage();

    VK_CHECK_RESULT(
        vkResetCommandPool(m_Device->GetVulkanDevice(), m_CommandBuffers[m_CurrentBufferIndex].CommandPool, 0));
}

void VulkanSwapChain::Present()
{
    VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_Semaphores.PresentComplete,
        .pWaitDstStageMask = &waitStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_CommandBuffers[m_CurrentBufferIndex].CommandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m_Semaphores.RenderComplete,
    };

    VK_CHECK_RESULT(vkResetFences(m_Device->GetVulkanDevice(), 1, &m_WaitFences[m_CurrentBufferIndex]));
    VK_CHECK_RESULT(vkQueueSubmit(m_Device->GetGraphicsQueue(), 1, &submitInfo, m_WaitFences[m_CurrentBufferIndex]));

    // Present the current buffer to the swap chain
    // Pass the semaphore signaled by the command buffer submission from the submit info as the wait semaphore for swap
    // chain presentation This ensures that the image is not presented to the windowing system until all commands have
    // been submitted
    VkResult result;
    {
        VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = NULL,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_Semaphores.RenderComplete,
            .swapchainCount = 1,
            .pSwapchains = &m_SwapChain,
            .pImageIndices = &m_CurrentImageIndex,
        };
        result = fpQueuePresentKHR(m_Device->GetGraphicsQueue(), &presentInfo);
    }

    if (result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            OnResize(m_Width, m_Height);
        } else {
            VK_CHECK_RESULT(result);
        }
    }

    {
        const auto &config = Renderer::GetConfig();
        m_CurrentBufferIndex = (m_CurrentBufferIndex + 1) % config.FramesInFlight;
        // Make sure the frame we're requesting has finished rendering
        VK_CHECK_RESULT(
            vkWaitForFences(m_Device->GetVulkanDevice(), 1, &m_WaitFences[m_CurrentBufferIndex], VK_TRUE, UINT64_MAX));
    }
}

uint32_t VulkanSwapChain::AcquireNextImage()
{
    uint32_t imageIndex;
    VK_CHECK_RESULT(fpAcquireNextImageKHR(m_Device->GetVulkanDevice(), m_SwapChain, UINT64_MAX,
                                          m_Semaphores.PresentComplete, (VkFence) nullptr, &imageIndex));
    return imageIndex;
}

void VulkanSwapChain::FindImageFormatAndColorSpace()
{
    VkPhysicalDevice physicalDevice = m_Device->GetPhysicalDevice()->GetVulkanPhysicalDevice();

    // Get list of supported surface formats
    uint32_t formatCount;
    VK_CHECK_RESULT(fpGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, NULL));
    check(formatCount > 0);

    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    VK_CHECK_RESULT(
        fpGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, surfaceFormats.data()));

    // If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
    // there is no preferered format, so we assume VK_FORMAT_B8G8R8A8_UNORM
    if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED)) {
        m_ColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
        m_ColorSpace = surfaceFormats[0].colorSpace;
    } else {
        // iterate over the list of available surface format and
        // check for the presence of VK_FORMAT_B8G8R8A8_UNORM
        bool found_B8G8R8A8_UNORM = false;
        for (auto &&surfaceFormat: surfaceFormats) {
            if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM) {
                m_ColorFormat = surfaceFormat.format;
                m_ColorSpace = surfaceFormat.colorSpace;
                found_B8G8R8A8_UNORM = true;
                break;
            }
        }

        // in case VK_FORMAT_B8G8R8A8_UNORM is not available
        // select the first available color format
        if (!found_B8G8R8A8_UNORM) {
            m_ColorFormat = surfaceFormats[0].format;
            m_ColorSpace = surfaceFormats[0].colorSpace;
        }
    }
}
}    // namespace Raphael

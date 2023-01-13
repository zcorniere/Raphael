#include "Engine/Renderer/Vulkan/VulkanRenderer.hxx"

#include "Engine/Renderer/Vulkan/VulkanContext.hxx"

namespace Raphael
{
namespace Utils
{

    static constexpr std::string_view VulkanVendorIDToString(std::uint32_t vendorID)
    {
        switch (vendorID) {
            case 0x10DE: return "NVIDIA";
            case 0x1002: return "AMD";
            case 0x8086: return "INTEL";
        }
        return "Unknown";
    }

}    // namespace Utils

struct VulkanRendererData {
    RendererCapabilities RenderCapabilities;

    std::vector<uint32_t> DescriptorPoolAllocationCount;
    std::vector<VkDescriptorPool> DescriptorPools;

    int32_t DrawCallCount = 0;
};

static VulkanRendererData *s_Data = nullptr;

void VulkanRenderer::Init()
{
    s_Data = new VulkanRendererData();

    const auto &config = Renderer::GetConfig();
    s_Data->DescriptorPools.resize(config.FramesInFlight);
    s_Data->DescriptorPoolAllocationCount.resize(config.FramesInFlight);

    auto &caps = s_Data->RenderCapabilities;
    auto &properties = VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->GetProperties();
    caps.Vendor = Utils::VulkanVendorIDToString(properties.vendorID);
    caps.Device = properties.deviceName;
    caps.Version = std::to_string(properties.driverVersion);

    Utils::DumpGPUInfo();

    // Create descriptor pools
    Renderer::Submit([] mutable {
        // Create Descriptor Pool
        VkDescriptorPoolSize pool_sizes[] = {
            {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000},
        };
        VkDescriptorPoolCreateInfo pool_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = 100000,
            .poolSizeCount = std::size(pool_sizes),
            .pPoolSizes = pool_sizes,
        };
        VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
        uint32_t framesInFlight = Renderer::GetConfig().FramesInFlight;
        for (uint32_t i = 0; i < framesInFlight; i++) {
            VK_CHECK_RESULT(vkCreateDescriptorPool(device, &pool_info, nullptr, &s_Data->DescriptorPools[i]));
            s_Data->DescriptorPoolAllocationCount[i] = 0;
        }
    });
}

void VulkanRenderer::Shutdown()
{
    VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
    vkDeviceWaitIdle(device);

    delete s_Data;
    s_Data = nullptr;
}

void VulkanRenderer::BeginFrame()
{
    Renderer::Submit([] {
        VulkanSwapChain &swapChain = Application::Get().GetWindow().GetSwapChain();

        // Reset descriptor pools here
        VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
        uint32_t bufferIndex = swapChain.GetCurrentBufferIndex();
        vkResetDescriptorPool(device, s_Data->DescriptorPools[bufferIndex], 0);
        memset(s_Data->DescriptorPoolAllocationCount.data(), 0,
               s_Data->DescriptorPoolAllocationCount.size() * sizeof(uint32_t));

        s_Data->DrawCallCount = 0;
    });
}

void VulkanRenderer::EndFrame()
{
}

const RendererCapabilities &VulkanRenderer::GetCapabilities() const
{
    return s_Data->RenderCapabilities;
}

namespace Utils
{

    void InsertImageMemoryBarrier(VkCommandBuffer cmdbuffer, VkImage image, VkAccessFlags srcAccessMask,
                                  VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout,
                                  VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask,
                                  VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange)
    {
        VkImageMemoryBarrier imageMemoryBarrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = srcAccessMask,
            .dstAccessMask = dstAccessMask,
            .oldLayout = oldImageLayout,
            .newLayout = newImageLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = subresourceRange,
        };

        vkCmdPipelineBarrier(cmdbuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
    }

    void SetImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkImageLayout oldImageLayout,
                        VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange,
                        VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
    {
        // Create an image barrier object
        VkImageMemoryBarrier imageMemoryBarrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .oldLayout = oldImageLayout,
            .newLayout = newImageLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = subresourceRange,
        };

        // Source layouts (old)
        // Source access mask controls actions that have to be finished on the old layout
        // before it will be transitioned to the new layout
        switch (oldImageLayout) {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                // Image layout is undefined (or does not matter)
                // Only valid as initial layout
                // No flags required, listed only for completeness
                imageMemoryBarrier.srcAccessMask = 0;
                break;

            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                // Image is preinitialized
                // Only valid as initial layout for linear images, preserves memory contents
                // Make sure host writes have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image is a color attachment
                // Make sure any writes to the color buffer have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image is a depth/stencil attachment
                // Make sure any writes to the depth/stencil buffer have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image is a transfer source
                // Make sure any reads from the image have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image is a transfer destination
                // Make sure any writes to the image have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image is read by a shader
                // Make sure any shader reads from the image have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                // Other source layouts aren't handled (yet)
                break;
        }

        // Target layouts (new)
        // Destination access mask controls the dependency for the new image layout
        switch (newImageLayout) {
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image will be used as a transfer destination
                // Make sure any writes to the image have been finished
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image will be used as a transfer source
                // Make sure any reads from the image have been finished
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image will be used as a color attachment
                // Make sure any writes to the color buffer have been finished
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image layout will be used as a depth/stencil attachment
                // Make sure any writes to depth/stencil buffer have been finished
                imageMemoryBarrier.dstAccessMask =
                    imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image will be read in a shader (sampler, input attachment)
                // Make sure any writes to the image have been finished
                if (imageMemoryBarrier.srcAccessMask == 0) {
                    imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
                }
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                // Other source layouts aren't handled (yet)
                break;
        }

        // Put barrier inside setup command buffer
        vkCmdPipelineBarrier(cmdbuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
    }

    void SetImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkImageAspectFlags aspectMask,
                        VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask,
                        VkPipelineStageFlags dstStageMask)
    {
        VkImageSubresourceRange subresourceRange{
            .aspectMask = aspectMask,
            .baseMipLevel = 0,
            .levelCount = 1,
            .layerCount = 1,
        };
        SetImageLayout(cmdbuffer, image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
    }

}    // namespace Utils

}    // namespace Raphael

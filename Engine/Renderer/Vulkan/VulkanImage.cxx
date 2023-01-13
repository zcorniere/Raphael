#include "Engine/Renderer/Vulkan/VulkanImage.hxx"

#include "Engine/Renderer/Vulkan/VulkanRenderer.hxx"

namespace Raphael
{

DECLARE_LOGGER_CATEGORY(Core, LogVulkanImage, Info);

static std::map<VkImage, WeakRef<VulkanImage2D>> s_ImageReferences;

VulkanImage2D::VulkanImage2D(const ImageSpecification &specification): m_Specification(specification)
{
    verifyAlways(m_Specification.Width > 0 && m_Specification.Height > 0);
    m_ImageData = Ref<Buffer<std::byte>>::Create();
}

VulkanImage2D::~VulkanImage2D()
{
    if (m_Info.Image) {
        const VulkanImageInfo &info = m_Info;
        Renderer::SubmitResourceFree([info, layerViews = m_PerLayerImageViews]() {
            const auto vulkanDevice = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
            vkDestroyImageView(vulkanDevice, info.ImageView, nullptr);
            vkDestroySampler(vulkanDevice, info.Sampler, nullptr);

            for (auto &view: layerViews) {
                if (view) vkDestroyImageView(vulkanDevice, view, nullptr);
            }

            VulkanAllocator allocator("VulkanImage2D");
            allocator.DestroyImage(info.Image, info.MemoryAlloc);
            s_ImageReferences.erase(info.Image);

            LOG(LogVulkanImage, Info, "VulkanImage2D::Release ImageView = {0}", (const void *)info.ImageView);
        });
        m_PerLayerImageViews.clear();
    }
}

void VulkanImage2D::Invalidate()
{
    Ref<VulkanImage2D> instance = this;
    Renderer::Submit([instance]() mutable { instance->RT_Invalidate(); });
}

void VulkanImage2D::Release()
{
    if (m_Info.Image == nullptr) return;

    Ref<VulkanImage2D> instance = this;
    const VulkanImageInfo &info = m_Info;
    Renderer::SubmitResourceFree(
        [instance, info, mipViews = m_PerMipImageViews, layerViews = m_PerLayerImageViews]() mutable {
            const auto vulkanDevice = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
            vkDestroyImageView(vulkanDevice, info.ImageView, nullptr);
            vkDestroySampler(vulkanDevice, info.Sampler, nullptr);

            for (auto &view: mipViews) {
                if (view.second) vkDestroyImageView(vulkanDevice, view.second, nullptr);
            }
            for (auto &view: layerViews) {
                if (view) vkDestroyImageView(vulkanDevice, view, nullptr);
            }
            VulkanAllocator allocator("VulkanImage2D");
            allocator.DestroyImage(info.Image, info.MemoryAlloc);
            s_ImageReferences.erase(info.Image);
        });
    m_Info.Image = nullptr;
    m_Info.ImageView = nullptr;
    m_Info.Sampler = nullptr;
    m_PerLayerImageViews.clear();
    m_PerMipImageViews.clear();
}

void VulkanImage2D::RT_Invalidate()
{
    verifyAlways(m_Specification.Width > 0 && m_Specification.Height > 0);

    // Try release first if necessary
    Release();

    VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
    VulkanAllocator allocator("Image2D");

    VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;    // TODO: this (probably) shouldn't be implied
    if (m_Specification.Usage == ImageUsage::Attachment) {
        if (Utils::IsDepthFormat(m_Specification.Format))
            usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        else
            usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    } else if (m_Specification.Usage == ImageUsage::Texture) {
        usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    } else if (m_Specification.Usage == ImageUsage::Storage) {
        usage |= VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    VkImageAspectFlags aspectMask =
        Utils::IsDepthFormat(m_Specification.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    if (m_Specification.Format == ImageFormat::DEPTH24STENCIL8) aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

    VkFormat vulkanFormat = Utils::VulkanImageFormat(m_Specification.Format);

    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = vulkanFormat;
    imageCreateInfo.extent.width = m_Specification.Width;
    imageCreateInfo.extent.height = m_Specification.Height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = m_Specification.Mips;
    imageCreateInfo.arrayLayers = m_Specification.Layers;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = usage;
    m_Info.MemoryAlloc = allocator.AllocateImage(imageCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_Info.Image);
    s_ImageReferences[m_Info.Image] = this;
    VKUtils::SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_IMAGE, m_Specification.DebugName, m_Info.Image);

    // Create a default image view
    VkImageViewCreateInfo imageViewCreateInfo = {};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.viewType = m_Specification.Layers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = vulkanFormat;
    imageViewCreateInfo.flags = 0;
    imageViewCreateInfo.subresourceRange = {};
    imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = m_Specification.Mips;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = m_Specification.Layers;
    imageViewCreateInfo.image = m_Info.Image;
    VK_CHECK_RESULT(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_Info.ImageView));
    VKUtils::SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_IMAGE_VIEW,
                                     fmt::format("{} default image view", m_Specification.DebugName), m_Info.ImageView);

    // TODO: Renderer should contain some kind of sampler cache
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.maxAnisotropy = 1.0f;
    if (Utils::IsIntegerBased(m_Specification.Format)) {
        samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
        samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    } else {
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }

    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
    samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.maxAnisotropy = 1.0f;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = 100.0f;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    VK_CHECK_RESULT(vkCreateSampler(device, &samplerCreateInfo, nullptr, &m_Info.Sampler));
    VKUtils::SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_SAMPLER,
                                     fmt::format("{} default sampler", m_Specification.DebugName), m_Info.Sampler);

    if (m_Specification.Usage == ImageUsage::Storage) {
        // Transition image to GENERAL layout
        VkCommandBuffer commandBuffer = VulkanContext::GetCurrentDevice()->GetCommandBuffer(true);

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = m_Specification.Mips;
        subresourceRange.layerCount = m_Specification.Layers;

        Utils::InsertImageMemoryBarrier(commandBuffer, m_Info.Image, 0, 0, VK_IMAGE_LAYOUT_UNDEFINED,
                                        VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, subresourceRange);

        VulkanContext::GetCurrentDevice()->FlushCommandBuffer(commandBuffer);
    }

    UpdateDescriptor();
}

void VulkanImage2D::CreatePerLayerImageViews()
{
    Ref<VulkanImage2D> instance = this;
    Renderer::Submit([instance]() mutable { instance->RT_CreatePerLayerImageViews(); });
}

void VulkanImage2D::RT_CreatePerLayerImageViews()
{
    check(m_Specification.Layers > 1);

    VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

    VkImageAspectFlags aspectMask =
        Utils::IsDepthFormat(m_Specification.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    if (m_Specification.Format == ImageFormat::DEPTH24STENCIL8) aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

    const VkFormat vulkanFormat = Utils::VulkanImageFormat(m_Specification.Format);

    m_PerLayerImageViews.resize(m_Specification.Layers);
    for (uint32_t layer = 0; layer < m_Specification.Layers; layer++) {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = vulkanFormat;
        imageViewCreateInfo.flags = 0;
        imageViewCreateInfo.subresourceRange = {};
        imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = m_Specification.Mips;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = layer;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.image = m_Info.Image;
        VK_CHECK_RESULT(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_PerLayerImageViews[layer]));
        VKUtils::SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_IMAGE_VIEW,
                                         fmt::format("{} image view layer: {}", m_Specification.DebugName, layer),
                                         m_PerLayerImageViews[layer]);
    }
}

VkImageView VulkanImage2D::GetMipImageView(uint32_t mip)
{
    if (m_PerMipImageViews.find(mip) == m_PerMipImageViews.end()) {
        Ref<VulkanImage2D> instance = this;
        Renderer::Submit([instance, mip]() mutable { instance->RT_GetMipImageView(mip); });
        return nullptr;
    }

    return m_PerMipImageViews.at(mip);
}

VkImageView VulkanImage2D::RT_GetMipImageView(const uint32_t mip)
{
    if (m_PerMipImageViews.find(mip) == m_PerMipImageViews.end()) {
        VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

        VkImageAspectFlags aspectMask =
            Utils::IsDepthFormat(m_Specification.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        if (m_Specification.Format == ImageFormat::DEPTH24STENCIL8) aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

        VkFormat vulkanFormat = Utils::VulkanImageFormat(m_Specification.Format);

        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = vulkanFormat;
        imageViewCreateInfo.flags = 0;
        imageViewCreateInfo.subresourceRange = {};
        imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
        imageViewCreateInfo.subresourceRange.baseMipLevel = mip;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.image = m_Info.Image;

        VK_CHECK_RESULT(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_PerMipImageViews[mip]));
        VKUtils::SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_IMAGE_VIEW,
                                         fmt::format("{} image view mip: {}", m_Specification.DebugName, mip),
                                         m_PerMipImageViews[mip]);
    }
    return m_PerMipImageViews.at(mip);
}

void VulkanImage2D::RT_CreatePerSpecificLayerImageViews(const std::vector<uint32_t> &layerIndices)
{
    check(m_Specification.Layers > 1);

    VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

    VkImageAspectFlags aspectMask =
        Utils::IsDepthFormat(m_Specification.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    if (m_Specification.Format == ImageFormat::DEPTH24STENCIL8) aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

    const VkFormat vulkanFormat = Utils::VulkanImageFormat(m_Specification.Format);

    // check(m_PerLayerImageViews.size() == m_Specification.Layers);
    if (m_PerLayerImageViews.empty()) m_PerLayerImageViews.resize(m_Specification.Layers);

    for (uint32_t layer: layerIndices) {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = vulkanFormat;
        imageViewCreateInfo.flags = 0;
        imageViewCreateInfo.subresourceRange = {};
        imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = m_Specification.Mips;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = layer;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.image = m_Info.Image;
        VK_CHECK_RESULT(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_PerLayerImageViews[layer]));
        VKUtils::SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_IMAGE_VIEW,
                                         fmt::format("{} image view layer: {}", m_Specification.DebugName, layer),
                                         m_PerLayerImageViews[layer]);
    }
}

void VulkanImage2D::UpdateDescriptor()
{
    if (m_Specification.Format == ImageFormat::DEPTH24STENCIL8 || m_Specification.Format == ImageFormat::DEPTH32F ||
        m_Specification.Format == ImageFormat::DEPTH32FSTENCIL8UINT)
        m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    else if (m_Specification.Usage == ImageUsage::Storage)
        m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    else
        m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    if (m_Specification.Usage == ImageUsage::Storage) m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    m_DescriptorImageInfo.imageView = m_Info.ImageView;
    m_DescriptorImageInfo.sampler = m_Info.Sampler;

    // LOG(LogVulkanImage, Info, "VulkanImage2D::UpdateDescriptor to ImageView = {0}", (const void*)m_Info.ImageView);
}

const std::map<VkImage, WeakRef<VulkanImage2D>> &VulkanImage2D::GetImageRefs()
{
    return s_ImageReferences;
}

}    // namespace Raphael

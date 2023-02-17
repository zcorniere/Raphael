#include "Engine/Renderer/Vulkan/VulkanResources.hxx"

#include "Engine/Renderer/Vulkan/VulkanDevice.hxx"
#include "Engine/Renderer/Vulkan/VulkanUtils.hxx"

VulkanTexture::VulkanTexture(Ref<VulkanDevice> InDevice, const RHITextureCreateDesc &InDesc)
    : RHITexture(InDesc), Description(InDesc), Device(InDevice), Allocation(nullptr)
{
    SetName(InDesc.DebugName);
    const VkPhysicalDeviceProperties &DeviceProperties = Device->GetDeviceProperties();

    VkImageCreateInfo ImageCreateInfo;
    ZeroVulkanStruct(ImageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
    ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    ImageCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    ImageCreateInfo.extent = {
        .width = InDesc.Extent.x,
        .height = InDesc.Extent.y,
        .depth = InDesc.Depth,
    };
    ImageCreateInfo.mipLevels = 1;
    ImageCreateInfo.arrayLayers = 1;
    ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    ImageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ImageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    ImageCreateInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;

    const VkImageViewType ResourceType = TextureDimensionToVkImageViewType(InDesc.Dimension);
    switch (ResourceType) {
        case VK_IMAGE_VIEW_TYPE_2D:
            ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
            check(InDesc.Extent.x <= DeviceProperties.limits.maxImageDimension2D);
            check(InDesc.Extent.y <= DeviceProperties.limits.maxImageDimension2D);
            break;
        default: checkNoEntry();
    }

    switch (InDesc.NumSamples) {
        case 1: ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT; break;
        case 2: ImageCreateInfo.samples = VK_SAMPLE_COUNT_2_BIT; break;
        case 4: ImageCreateInfo.samples = VK_SAMPLE_COUNT_4_BIT; break;
        case 8: ImageCreateInfo.samples = VK_SAMPLE_COUNT_8_BIT; break;
        case 16: ImageCreateInfo.samples = VK_SAMPLE_COUNT_16_BIT; break;
        case 32: ImageCreateInfo.samples = VK_SAMPLE_COUNT_32_BIT; break;
        case 64: ImageCreateInfo.samples = VK_SAMPLE_COUNT_64_BIT; break;
        default: checkNoEntry(); break;
    }

    VK_CHECK_RESULT(VulkanAPI::vkCreateImage(Device->GetInstanceHandle(), &ImageCreateInfo, nullptr, &Image));
    VulkanAPI::vkGetImageMemoryRequirements(Device->GetInstanceHandle(), Image, &MemoryRequirements);

    VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_IMAGE, Image, "{:s}:(VulkanTexture){:p}",
                          InDesc.DebugName.empty() ? "?" : InDesc.DebugName, (void *)this);

    Device->GetMemoryManager()->Alloc(MemoryRequirements.size, VMA_MEMORY_USAGE_GPU_ONLY, false);
    Allocation->BindImage(Image);
}

VulkanTexture::~VulkanTexture()
{
    VulkanAPI::vkDestroyImage(Device->GetInstanceHandle(), Image, nullptr);
}

void VulkanTextureView::Create(Ref<VulkanDevice> &Device, VkImage InImage, VkImageViewType ViewType,
                               VkImageAspectFlags AspectFlags, VkFormat Format, uint32 FirstMip, uint32 NumMips)
{
    VkImageViewCreateInfo ViewInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = InImage,
        .viewType = ViewType,
        .format = Format,
        .subresourceRange =
            {
                .aspectMask = AspectFlags,
                .baseMipLevel = FirstMip,
                .levelCount = NumMips,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };

    VK_CHECK_RESULT(VulkanAPI::vkCreateImageView(Device->GetInstanceHandle(), &ViewInfo, nullptr, &View));

    Image = InImage;
}

void VulkanTextureView::Destroy(Ref<VulkanDevice> &Device)
{
    if (View) { VulkanAPI::vkDestroyImageView(Device->GetInstanceHandle(), View, nullptr); }
}

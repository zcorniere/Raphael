#include "VulkanRHI/Resources/VulkanTexture.hxx"

#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanMemoryManager.hxx"
#include "VulkanRHI/VulkanUtils.hxx"

namespace VulkanRHI
{

VulkanTexture::VulkanTexture(VulkanDevice* InDevice, const RHITextureSpecification& InDesc)
    : RHITexture(InDesc),
      Device(InDevice),
      Description(InDesc),
      Allocation(nullptr),
      Image(VK_NULL_HANDLE),
      Layout(VK_IMAGE_LAYOUT_UNDEFINED),
      View(VK_NULL_HANDLE)
{
    const VkPhysicalDeviceProperties& DeviceProperties = Device->GetDeviceProperties();

    VkImageCreateInfo ImageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = TextureDimensionToVkImageType(InDesc.Dimension),
        .format = ImageFormatToFormat(InDesc.Format),
        .extent =
            {
                .width = InDesc.Extent.x,
                .height = InDesc.Extent.y,
                .depth = InDesc.Depth,
            },
        .mipLevels = InDesc.NumMips,
        .arrayLayers = 1,
        .usage = TextureUsageFlagsToVkImageUsageFlags(Description.Flags),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = Layout,
    };

    const VkImageViewType ResourceImageView = TextureDimensionToVkImageViewType(InDesc.Dimension);
    switch (ResourceImageView) {
        case VK_IMAGE_VIEW_TYPE_2D:
            ImageCreateInfo.imageType = TextureDimensionToVkImageType(InDesc.Dimension);
            check(InDesc.Extent.x <= DeviceProperties.limits.maxImageDimension2D);
            check(InDesc.Extent.y <= DeviceProperties.limits.maxImageDimension2D);
            break;
        default:
            checkNoEntry() break;
    }

    switch (InDesc.NumSamples) {
        case 1:
            ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            break;
        case 2:
            ImageCreateInfo.samples = VK_SAMPLE_COUNT_2_BIT;
            break;
        case 4:
            ImageCreateInfo.samples = VK_SAMPLE_COUNT_4_BIT;
            break;
        case 8:
            ImageCreateInfo.samples = VK_SAMPLE_COUNT_8_BIT;
            break;
        case 16:
            ImageCreateInfo.samples = VK_SAMPLE_COUNT_16_BIT;
            break;
        case 32:
            ImageCreateInfo.samples = VK_SAMPLE_COUNT_32_BIT;
            break;
        case 64:
            ImageCreateInfo.samples = VK_SAMPLE_COUNT_64_BIT;
            break;
        default:
            checkNoEntry();
            break;
    }

    VK_CHECK_RESULT(VulkanAPI::vkCreateImage(Device->GetHandle(), &ImageCreateInfo, VULKAN_CPU_ALLOCATOR, &Image));
    VulkanAPI::vkGetImageMemoryRequirements(Device->GetHandle(), Image, &MemoryRequirements);

    Allocation = Device->GetMemoryManager()->Alloc(MemoryRequirements, VMA_MEMORY_USAGE_GPU_ONLY, false);
    Allocation->BindImage(Image);
}

VulkanTexture::~VulkanTexture()
{
    if (View) {
        VulkanAPI::vkDestroyImageView(Device->GetHandle(), View, VULKAN_CPU_ALLOCATOR);
    }
    Device->GetMemoryManager()->Free(Allocation);
    Allocation = nullptr;
    VulkanAPI::vkDestroyImage(Device->GetHandle(), Image, VULKAN_CPU_ALLOCATOR);
}

void VulkanTexture::SetName(std::string_view InName)
{
    RHIResource::SetName(InName);
    if (Image) {
        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_IMAGE, Image, "{:s}.Image", InName);
    }
    if (View) {
        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_IMAGE_VIEW, View, "{:s}.Image.View", InName);
    }
    if (Allocation) {
        Allocation->SetName(std::format("{:s}.Image.Memory", InName));
    }
}

VkImage VulkanTexture::GetImage() const
{
    return Image;
}

VkImageView VulkanTexture::GetImageView() const
{
    if (View != VK_NULL_HANDLE) {
        return View;
    }
    VkImageViewCreateInfo CreateInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = Image,
        .viewType = TextureDimensionToVkImageViewType(Description.Dimension),
        .format = ImageFormatToFormat(Description.Format),
        .components =
            {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
        .subresourceRange = Barrier::MakeSubresourceRange(TextureUsageFlagToVkImageAspectFlags(Description.Flags)),
    };
    VK_CHECK_RESULT(VulkanAPI::vkCreateImageView(Device->GetHandle(), &CreateInfo, VULKAN_CPU_ALLOCATOR, &View));
    VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_IMAGE_VIEW, View, "{:s} [View]", GetName());
    return View;
}

VkImageViewType VulkanTexture::GetViewType() const
{
    return TextureDimensionToVkImageViewType(Description.Dimension);
}

VkImageLayout VulkanTexture::GetLayout() const
{
    return Layout;
}

//////////////////// VulkanTextureView ////////////////////

void VulkanTextureView::Create(VulkanDevice* Device, VkImage InImage, VkImageViewType ViewType,
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

    VK_CHECK_RESULT(VulkanAPI::vkCreateImageView(Device->GetHandle(), &ViewInfo, VULKAN_CPU_ALLOCATOR, &View));

    Image = InImage;
}

void VulkanTextureView::Destroy(VulkanDevice* Device)
{
    if (View) {
        VulkanAPI::vkDestroyImageView(Device->GetHandle(), View, VULKAN_CPU_ALLOCATOR);
        Image = VK_NULL_HANDLE;
        View = VK_NULL_HANDLE;
    }
}

}    // namespace VulkanRHI

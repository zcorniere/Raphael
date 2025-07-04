#include "VulkanRHI/Resources/VulkanTexture.hxx"

#include "Engine/Core/RHI/RHICommandList.hxx"
#include "VulkanRHI/VulkanCommandContext.hxx"
#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanMemoryManager.hxx"
#include "VulkanRHI/VulkanUtils.hxx"

namespace VulkanRHI
{

RVulkanTexture::RVulkanTexture(FVulkanDevice* InDevice, const FRHITextureSpecification& InDesc)
    : Super(InDesc)
    , IDeviceChild(InDevice)
    , Allocation(nullptr)
    , Image(VK_NULL_HANDLE)
    , Layout(VK_IMAGE_LAYOUT_UNDEFINED)
    , View(VK_NULL_HANDLE)
{
    CreateTexture();
}

RVulkanTexture::~RVulkanTexture()
{
    DestroyTexture();
}

void RVulkanTexture::SetName(std::string_view InName)
{
    Super::SetName(InName);
    if (Image)
    {
        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_IMAGE, Image, "{:s}.Image", InName);
    }
    if (View)
    {
        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_IMAGE_VIEW, View, "{:s}.Image.View", InName);
    }
    if (Allocation)
    {
        Allocation->SetName(std::format("{:s}.Image.Memory", InName));
    }
}

void RVulkanTexture::Invalidate()
{
    ENQUEUE_RENDER_COMMAND(InvalidateTexture)
    (
        [instance = WeakRef(this)](FFRHICommandList& CommandList) mutable
        {
            const VkImageLayout Layout = instance->GetLayout();
            instance->DestroyTexture();
            instance->CreateTexture();

            if (Layout != VK_IMAGE_LAYOUT_UNDEFINED)
            {
                FVulkanCommandContext* Context = CommandList.GetContext()->Cast<FVulkanCommandContext>();
                Context->SetLayout(instance.Raw(), Layout);
            }
        });
}

VkImage RVulkanTexture::GetImage() const
{
    return Image;
}

VkImageView RVulkanTexture::GetImageView() const
{
    if (View != VK_NULL_HANDLE)
    {
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
        .subresourceRange = FBarrier::MakeSubresourceRange(TextureUsageFlagToVkImageAspectFlags(Description.Flags)),
    };
    VK_CHECK_RESULT(VulkanAPI::vkCreateImageView(Device->GetHandle(), &CreateInfo, VULKAN_CPU_ALLOCATOR, &View));
    VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_IMAGE_VIEW, View, "{:s}.Image.View", GetName());
    return View;
}

VkImageViewType RVulkanTexture::GetViewType() const
{
    return TextureDimensionToVkImageViewType(Description.Dimension);
}

VkImageLayout RVulkanTexture::GetLayout() const
{
    return Layout;
}

void RVulkanTexture::SetLayout(FVulkanCmdBuffer* CmdBuffer, VkImageLayout NewLayout)
{
    VkImageAspectFlags AspectMask = 0;
    switch (Description.Format)
    {
        case EImageFormat::R8G8B8_SRGB:
        case EImageFormat::B8G8R8A8_SRGB:
        case EImageFormat::R8G8B8A8_SRGB:
            AspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
            break;
        case EImageFormat::D32_SFLOAT:
            AspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
            break;
    }
    VkImageSubresourceRange Range = FBarrier::MakeSubresourceRange(AspectMask, 0, Description.NumMips);
    VulkanSetImageLayout(CmdBuffer->GetHandle(), Image, Layout, NewLayout, Range);
    Layout = NewLayout;
}

void RVulkanTexture::CreateTexture()
{
    const VkPhysicalDeviceProperties& DeviceProperties = Device->GetDeviceProperties();

    VkImageCreateInfo ImageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = TextureDimensionToVkImageType(Description.Dimension),
        .format = ImageFormatToFormat(Description.Format),
        .extent =
            {
                .width = Description.Extent.x,
                .height = Description.Extent.y,
                .depth = Description.Depth,
            },
        .mipLevels = Description.NumMips,
        .arrayLayers = 1,
        .usage = TextureUsageFlagsToVkImageUsageFlags(Description.Flags),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = Layout,
    };

    const VkImageViewType ResourceImageView = TextureDimensionToVkImageViewType(Description.Dimension);
    switch (ResourceImageView)
    {
        case VK_IMAGE_VIEW_TYPE_2D:
            ImageCreateInfo.imageType = TextureDimensionToVkImageType(Description.Dimension);
            check(Description.Extent.x <= DeviceProperties.limits.maxImageDimension2D);
            check(Description.Extent.y <= DeviceProperties.limits.maxImageDimension2D);
            break;
        default:
            checkNoEntry() break;
    }

    switch (Description.NumSamples)
    {
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

void RVulkanTexture::DestroyTexture()
{
    RHI::DeferedDeletion(
        [View = this->View, Allocation = this->Allocation, Image = this->Image, Device = this->Device]() mutable
        {
            if (View)
            {
                VulkanAPI::vkDestroyImageView(Device->GetHandle(), View, VULKAN_CPU_ALLOCATOR);
            }
            Device->GetMemoryManager()->Free(Allocation);
            VulkanAPI::vkDestroyImage(Device->GetHandle(), Image, VULKAN_CPU_ALLOCATOR);
        });
    View = VK_NULL_HANDLE;
    Allocation = nullptr;
    Image = VK_NULL_HANDLE;
    Layout = VK_IMAGE_LAYOUT_UNDEFINED;
}

VkImageLayout RVulkanTexture::GetDefaultLayout() const
{
    switch (Description.Flags)
    {
        case ETextureUsageFlags::RenderTargetable:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case ETextureUsageFlags::DepthStencilTargetable:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case ETextureUsageFlags::SampleTargetable:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case ETextureUsageFlags::TransferTargetable:
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        default:
            return VK_IMAGE_LAYOUT_UNDEFINED;
    }
}

//////////////////// VulkanTextureView ////////////////////

void VulkanTextureView::Create(FVulkanDevice* Device, VkImage InImage, VkImageViewType ViewType,
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

void VulkanTextureView::Destroy(FVulkanDevice* Device)
{
    if (View)
    {
        RHI::DeferedDeletion([View = this->View, Device]
                             { VulkanAPI::vkDestroyImageView(Device->GetHandle(), View, VULKAN_CPU_ALLOCATOR); });
        // We don't own the image, so we don't destroy it
        Image = VK_NULL_HANDLE;
        View = VK_NULL_HANDLE;
    }
}

}    // namespace VulkanRHI

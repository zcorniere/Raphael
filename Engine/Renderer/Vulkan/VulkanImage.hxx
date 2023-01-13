#pragma once

#include "Engine/Renderer/Image.hxx"
#include "Engine/Renderer/Vulkan/VulkanContext.hxx"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Raphael
{

struct VulkanImageInfo {
    VkImage Image = nullptr;
    VkImageView ImageView = nullptr;
    VkSampler Sampler = nullptr;
    VmaAllocation MemoryAlloc = nullptr;
};

class VulkanImage2D : public Image2D
{
public:
    VulkanImage2D(const ImageSpecification &specification);
    virtual ~VulkanImage2D() override;

    virtual void Resize(const glm::uvec2 &size) override
    {
        Resize(size.x, size.y);
    }
    virtual void Resize(const uint32_t width, const uint32_t height) override
    {
        m_Specification.Width = width;
        m_Specification.Height = height;
        Invalidate();
    }
    virtual void Invalidate() override;
    virtual void Release() override;

    virtual uint32_t GetWidth() const override
    {
        return m_Specification.Width;
    }
    virtual uint32_t GetHeight() const override
    {
        return m_Specification.Height;
    }
    virtual glm::uvec2 GetSize() const override
    {
        return {m_Specification.Width, m_Specification.Height};
    }

    virtual float GetAspectRatio() const override
    {
        return (float)m_Specification.Width / (float)m_Specification.Height;
    }

    virtual ImageSpecification &GetSpecification() override
    {
        return m_Specification;
    }
    virtual const ImageSpecification &GetSpecification() const override
    {
        return m_Specification;
    }

    void RT_Invalidate();

    virtual void CreatePerLayerImageViews() override;
    void RT_CreatePerLayerImageViews();
    void RT_CreatePerSpecificLayerImageViews(const std::vector<uint32_t> &layerIndices);

    virtual VkImageView GetLayerImageView(uint32_t layer)
    {
        check(layer < m_PerLayerImageViews.size());
        return m_PerLayerImageViews[layer];
    }

    VkImageView GetMipImageView(uint32_t mip);
    VkImageView RT_GetMipImageView(uint32_t mip);

    VulkanImageInfo &GetImageInfo()
    {
        return m_Info;
    }
    const VulkanImageInfo &GetImageInfo() const
    {
        return m_Info;
    }

    const VkDescriptorImageInfo &GetDescriptorInfo() const
    {
        return m_DescriptorImageInfo;
    }

    virtual Ref<const Buffer<std::byte>> GetBuffer() const override
    {
        return m_ImageData;
    }
    virtual Ref<Buffer<std::byte>> GetBuffer() override
    {
        return m_ImageData;
    }

    virtual uint64_t GetHash() const override
    {
        return (uint64_t)m_Info.Image;
    }

    void UpdateDescriptor();

    // Debug
    static const std::map<VkImage, WeakRef<VulkanImage2D>> &GetImageRefs();

private:
    ImageSpecification m_Specification;

    Ref<Buffer<std::byte>> m_ImageData;

    VulkanImageInfo m_Info;

    std::vector<VkImageView> m_PerLayerImageViews;
    std::map<uint32_t, VkImageView> m_PerMipImageViews;
    VkDescriptorImageInfo m_DescriptorImageInfo = {};
};

namespace Utils
{

    inline VkFormat VulkanImageFormat(ImageFormat format)
    {
        switch (format) {
            case ImageFormat::RED8UN: return VK_FORMAT_R8_UNORM;
            case ImageFormat::RED8UI: return VK_FORMAT_R8_UINT;
            case ImageFormat::RED16UI: return VK_FORMAT_R16_UINT;
            case ImageFormat::RED32UI: return VK_FORMAT_R32_UINT;
            case ImageFormat::RED32F: return VK_FORMAT_R32_SFLOAT;
            case ImageFormat::RG8: return VK_FORMAT_R8G8_UNORM;
            case ImageFormat::RG16F: return VK_FORMAT_R16G16_SFLOAT;
            case ImageFormat::RG32F: return VK_FORMAT_R32G32_SFLOAT;
            case ImageFormat::RGBA: return VK_FORMAT_R8G8B8A8_UNORM;
            case ImageFormat::RGBA16F: return VK_FORMAT_R16G16B16A16_SFLOAT;
            case ImageFormat::RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;
            case ImageFormat::B10R11G11UF: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
            case ImageFormat::DEPTH32FSTENCIL8UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
            case ImageFormat::DEPTH32F: return VK_FORMAT_D32_SFLOAT;
            case ImageFormat::DEPTH24STENCIL8:
                return VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->GetDepthFormat();
            default: checkNoEntry(); return VK_FORMAT_UNDEFINED;
        }
    }

}    // namespace Utils

}    // namespace Raphael

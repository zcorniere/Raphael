#pragma once

#include "Engine/Core/Buffer.hxx"
#include "Engine/Core/RObject.hxx"

namespace Raphael
{

enum class ImageFormat {
    None = 0,
    RED8UN,
    RED8UI,
    RED16UI,
    RED32UI,
    RED32F,
    RG8,
    RG16F,
    RG32F,
    RGB,
    RGBA,
    RGBA16F,
    RGBA32F,

    B10R11G11UF,

    SRGB,

    DEPTH32FSTENCIL8UINT,
    DEPTH32F,
    DEPTH24STENCIL8,

    // Defaults
    Depth = DEPTH24STENCIL8,
};

enum class ImageUsage {
    None = 0,
    Texture,
    Attachment,
    Storage
};

enum class TextureWrap {
    None = 0,
    Clamp,
    Repeat
};

enum class TextureFilter {
    None = 0,
    Linear,
    Nearest,
    Cubic
};

enum class TextureType {
    None = 0,
    Texture2D,
    TextureCube
};

struct TextureProperties {
    std::string DebugName;
    TextureWrap SamplerWrap = TextureWrap::Repeat;
    TextureFilter SamplerFilter = TextureFilter::Linear;
    bool GenerateMips = true;
    bool SRGB = false;
    bool Storage = false;
};

struct ImageSpecification {
    std::string DebugName;

    ImageFormat Format = ImageFormat::RGBA;
    ImageUsage Usage = ImageUsage::Texture;
    uint32_t Width = 1;
    uint32_t Height = 1;
    uint32_t Mips = 1;
    uint32_t Layers = 1;
};

class Image : public RObject
{
public:
    virtual ~Image() = default;

    virtual void Resize(const uint32_t width, const uint32_t height) = 0;
    virtual void Invalidate() = 0;
    virtual void Release() = 0;

    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;
    virtual glm::uvec2 GetSize() const = 0;

    virtual float GetAspectRatio() const = 0;

    virtual ImageSpecification &GetSpecification() = 0;
    virtual const ImageSpecification &GetSpecification() const = 0;

    virtual Ref<const Buffer<std::byte>> GetBuffer() const = 0;
    virtual Ref<Buffer<std::byte>> GetBuffer() = 0;

    virtual void CreatePerLayerImageViews() = 0;

    virtual uint64_t GetHash() const = 0;

    // TODO: usage (eg. shader read)
};

class Image2D : public Image
{
public:
    static Ref<Image2D> Create(ImageSpecification specification);
    virtual void Resize(const glm::uvec2 &size) = 0;
};

namespace Utils
{

    inline uint32_t GetImageFormatBPP(ImageFormat format)
    {
        switch (format) {
            case ImageFormat::RED8UN: return 1;
            case ImageFormat::RED8UI: return 1;
            case ImageFormat::RED16UI: return 2;
            case ImageFormat::RED32UI: return 4;
            case ImageFormat::RED32F: return 4;
            case ImageFormat::RGB:
            case ImageFormat::SRGB: return 3;
            case ImageFormat::RGBA: return 4;
            case ImageFormat::RGBA16F: return 2 * 4;
            case ImageFormat::RGBA32F: return 4 * 4;
            case ImageFormat::B10R11G11UF: return 4;
            default: checkNoEntry(); return 0;
        }
    }

    inline bool IsIntegerBased(const ImageFormat format)
    {
        switch (format) {
            case ImageFormat::RED16UI:
            case ImageFormat::RED32UI:
            case ImageFormat::RED8UI:
            case ImageFormat::DEPTH32FSTENCIL8UINT: return true;
            case ImageFormat::DEPTH32F:
            case ImageFormat::RED8UN:
            case ImageFormat::RGBA32F:
            case ImageFormat::B10R11G11UF:
            case ImageFormat::RG16F:
            case ImageFormat::RG32F:
            case ImageFormat::RED32F:
            case ImageFormat::RG8:
            case ImageFormat::RGBA:
            case ImageFormat::RGBA16F:
            case ImageFormat::RGB:
            case ImageFormat::SRGB:
            case ImageFormat::DEPTH24STENCIL8: return false;
            default: checkNoEntry(); return false;
        }
    }

    inline uint32_t CalculateMipCount(uint32_t width, uint32_t height)
    {
        return (uint32_t)std::floor(std::log2(glm::min(width, height))) + 1;
    }

    inline uint32_t GetImageMemorySize(ImageFormat format, uint32_t width, uint32_t height)
    {
        return width * height * GetImageFormatBPP(format);
    }

    inline bool IsDepthFormat(ImageFormat format)
    {
        if (format == ImageFormat::DEPTH24STENCIL8 || format == ImageFormat::DEPTH32F ||
            format == ImageFormat::DEPTH32FSTENCIL8UINT)
            return true;
        return false;
    }

}    // namespace Utils

}    // namespace Raphael

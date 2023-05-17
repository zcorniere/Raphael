#pragma once

#include "Engine/Misc/EnumFlags.hxx"
#include "Engine/Core/RHI/RHIDefinitions.hxx"
#include "Engine/Core/RHI/RHIResource.hxx"

enum class ETextureCreateFlags {
    None = 0,
    RenderTargetable = BIT(0),
    ResolveTargetable = BIT(1),
    DepthStencilTargetable = BIT(2),
};
ENUM_CLASS_FLAGS(ETextureCreateFlags)

struct RHITextureCreateDesc {
    ETextureCreateFlags Flags = ETextureCreateFlags::None;
    EImageDimension Dimension = EImageDimension::Texture2D;
    EImageFormat Format = EImageFormat::R8G8B8A8_RGBA;

    glm::uvec2 Extent = {1, 1};
    uint32 Depth = 1;
    uint8 NumMips = 1;
    uint8 NumSamples = 1;
};

class RHITexture : public RHIResource
{
public:
    RHITexture(const RHITextureCreateDesc &InDesc): RHIResource(RHIResourceType::Texture), Description(InDesc) {}
    virtual ~RHITexture() {}

    const RHITextureCreateDesc &GetDescription() const { return Description; }

    // RHI specific function
    virtual void *GetNativeResource() const { return nullptr; }

    glm::ivec3 GetMipDimensions(uint8 MipIndex) const
    {
        const RHITextureCreateDesc &Desc = GetDescription();
        return glm::ivec3(std::max(Desc.Extent.x >> MipIndex, 1u), std::max(Desc.Extent.y >> MipIndex, 1u),
                          std::max(Desc.Depth >> MipIndex, 1u));
    }

private:
    const RHITextureCreateDesc Description;
};

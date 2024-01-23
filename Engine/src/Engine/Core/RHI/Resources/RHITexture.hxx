#pragma once

#include "Engine/Core/RHI/RHIDefinitions.hxx"
#include "Engine/Core/RHI/RHIResource.hxx"

#include <glm/vec3.hpp>

/// @brief Describe the texture to be created
struct RHITextureSpecification {
    ETextureUsageFlags Flags = ETextureUsageFlags::TransferTargetable;
    EImageDimension Dimension = EImageDimension::Texture2D;
    EImageFormat Format = EImageFormat::R8G8B8A8_SRGB;

    glm::uvec2 Extent = {1, 1};
    uint32 Depth = 1;
    uint8 NumMips = 1;
    uint8 NumSamples = 1;

    glm::vec4 ClearColor = {0.0f, 0.0f, 0.0f, 1.0f};

    std::string Name;

    bool operator==(const RHITextureSpecification&) const = default;
};

/// @brief Represent a Texture used by the RHI
class RHITexture : public RHIResource
{
public:
    explicit RHITexture(const RHITextureSpecification& InDesc)
        : RHIResource(RHIResourceType::Texture), Description(InDesc)
    {
    }
    virtual ~RHITexture() = default;

    virtual void Resize(const glm::uvec2& Size);
    virtual void Invalidate() = 0;

    const RHITextureSpecification& GetDescription() const
    {
        return Description;
    }

    glm::ivec3 GetMipDimensions(uint8 MipIndex) const
    {
        const RHITextureSpecification& Desc = GetDescription();
        return glm::ivec3(std::max(Desc.Extent.x >> MipIndex, 1u), std::max(Desc.Extent.y >> MipIndex, 1u),
                          std::max(Desc.Depth >> MipIndex, 1u));
    }

protected:
    RHITextureSpecification Description;
};

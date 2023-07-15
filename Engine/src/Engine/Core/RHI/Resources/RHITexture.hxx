#pragma once

#include "Engine/Core/RHI/RHIDefinitions.hxx"
#include "Engine/Core/RHI/RHIResource.hxx"
#include "Engine/Misc/EnumFlags.hxx"

#include <glm/vec3.hpp>

/// @brief Represent a Texture used by the RHI
class RHITexture : public RHIResource
{
public:
    RHITexture(const RHITextureCreateDesc& InDesc): RHIResource(RHIResourceType::Texture), Description(InDesc)
    {
    }
    virtual ~RHITexture()
    {
    }

    const RHITextureCreateDesc& GetDescription() const
    {
        return Description;
    }

    glm::ivec3 GetMipDimensions(uint8 MipIndex) const
    {
        const RHITextureCreateDesc& Desc = GetDescription();
        return glm::ivec3(std::max(Desc.Extent.x >> MipIndex, 1u), std::max(Desc.Extent.y >> MipIndex, 1u),
                          std::max(Desc.Depth >> MipIndex, 1u));
    }

private:
    const RHITextureCreateDesc Description;
};

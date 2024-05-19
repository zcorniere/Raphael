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
        : RHIResource(ERHIResourceType::Texture), Description(InDesc)
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

/// @brief Group up information about the textures that is about to be used in a render
struct RHIRenderTarget {
    /// The texture to render to
    Ref<RHITexture> Texture;

    /// The color to clear the texture with
    glm::vec4 ClearColor = glm::vec4(0.0f);

    /// How the RHI should handle the texture load operation
    ERenderTargetLoadAction LoadAction = ERenderTargetLoadAction::NoAction;
    /// How the RHI should handle the texture store operation
    ERenderTargetStoreAction StoreAction = ERenderTargetStoreAction::NoAction;

    bool operator==(const RHIRenderTarget&) const = default;
};

struct RHIRenderPassDescription {
    glm::ivec2 RenderAreaLocation = glm::ivec2(0);
    glm::uvec2 RenderAreaSize;

    Array<RHIRenderTarget> ColorTargets = {};
    std::optional<RHIRenderTarget> DepthTarget = std::nullopt;

    bool operator==(const RHIRenderPassDescription&) const = default;
};

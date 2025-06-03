#pragma once

#include "Engine/Core/RHI/RHIDefinitions.hxx"
#include "Engine/Core/RHI/RHIResource.hxx"

/// @brief Describe the texture to be created
struct FRHITextureSpecification
{
    ETextureUsageFlags Flags = ETextureUsageFlags::TransferTargetable;
    EImageDimension Dimension = EImageDimension::Texture2D;
    EImageFormat Format = EImageFormat::R8G8B8A8_SRGB;

    UVector2 Extent = {1, 1};
    uint32 Depth = 1;
    uint8 NumMips = 1;
    uint8 NumSamples = 1;

    FVector4 ClearColor = {0.0f, 0.0f, 0.0f, 1.0f};

    std::string Name;

    bool operator==(const FRHITextureSpecification&) const = default;
};

/// @brief Represent a Texture used by the RHI
class RRHITexture : public RRHIResource
{
    RTTI_DECLARE_TYPEINFO(RRHITexture, RRHIResource);

public:
    explicit RRHITexture(const FRHITextureSpecification& InDesc)
        : RRHIResource(ERHIResourceType::Texture)
        , Description(InDesc)
    {
    }
    virtual ~RRHITexture() = default;

    void Resize(const UVector2& Size)
    {
        Description.Extent = Size;
        Invalidate();
    }
    virtual void Invalidate() = 0;

    const FRHITextureSpecification& GetDescription() const
    {
        return Description;
    }

    IVector3 GetMipDimensions(uint8 MipIndex) const
    {
        const FRHITextureSpecification& Desc = GetDescription();
        return IVector3(std::max(Desc.Extent.x >> MipIndex, 1u), std::max(Desc.Extent.y >> MipIndex, 1u),
                        std::max(Desc.Depth >> MipIndex, 1u));
    }

protected:
    FRHITextureSpecification Description;
};

/// @brief Group up information about the textures that is about to be used in a render
struct FRHIRenderTarget
{
    /// The texture to render to
    Ref<RRHITexture> Texture;

    /// The color to clear the texture with
    FVector4 ClearColor = FVector4(0.0f);

    /// How the RHI should handle the texture load operation
    ERenderTargetLoadAction LoadAction = ERenderTargetLoadAction::NoAction;
    /// How the RHI should handle the texture store operation
    ERenderTargetStoreAction StoreAction = ERenderTargetStoreAction::NoAction;

    bool operator==(const FRHIRenderTarget&) const = default;
};

struct FRHIRenderPassDescription
{
    IVector2 RenderAreaLocation = IVector2(0);
    UVector2 RenderAreaSize;

    TArray<FRHIRenderTarget> ColorTargets = {};
    std::optional<FRHIRenderTarget> DepthTarget = std::nullopt;

    bool operator==(const FRHIRenderPassDescription&) const = default;
};

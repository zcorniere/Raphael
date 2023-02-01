#pragma once

#include "Engine/Core/Buffer.hxx"
#include "Engine/Misc/EnumFlags.hxx"

namespace Raphael::RHI
{

enum class RHIResourceType : uint8 {
    None,

    Buffer,
    Texture,

    MAX_VALUE,
};

enum class TextureCreateFlags {
    None = 0,
    RenderTargetable = BIT(0),
    ResolveTargetable = BIT(1),
    DepthStencilTargetable = BIT(2),
};
ENUM_CLASS_FLAGS(TextureCreateFlags)

class RHITextureCreateDesc
{
public:
    static RHITextureCreateDesc Create(std::string_view InDebugName);

public:
    RHITextureCreateDesc(std::string_view InDebugName): DebugName(InDebugName)
    {
    }

    constexpr uint8 GetMaxMipLevel()
    {
        return std::floor(std::log2(std::max(Extent.x, Extent.y))) + 1;
    }

    const std::string DebugName;
    Ref<Buffer> InitialData = nullptr;

    TextureCreateFlags Flags = TextureCreateFlags::None;
    glm::uvec2 Extent = {1, 1};
    uint32 Depth = 1;
    uint8 NumMips = 1;
    uint8 NumSamples = 0;

private:
    static bool Validate(const RHITextureCreateDesc &Desc, std::string_view Name);
};

}    // namespace Raphael::RHI

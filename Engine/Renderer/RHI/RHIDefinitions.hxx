#pragma once

#include "Engine/Core/Buffer.hxx"
#include "Engine/Misc/EnumFlags.hxx"

namespace Raphael::RHI
{

enum class RHIResourceType : uint8 {
    None,

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

enum class TextureDimension {
    Texture2D,
};

struct RHITextureCreateDesc {
    const std::string DebugName;
    Ref<Buffer> InitialData = nullptr;

    TextureCreateFlags Flags = TextureCreateFlags::None;
    TextureDimension Dimension = TextureDimension::Texture2D;

    glm::uvec2 Extent = {1, 1};
    uint32 Depth = 1;
    uint8 NumMips = 1;
    uint8 NumSamples = 0;
};

}    // namespace Raphael::RHI

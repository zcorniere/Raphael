#pragma once

#include "Engine/Core/Buffer.hxx"
#include "Engine/Misc/EnumFlags.hxx"

enum class RHIResourceType : uint8 {
    None,

    Texture,
    Shader,
    Viewport,

    MAX_VALUE,
};

enum class RHIShaderType : uint8 {
    Vertex,
    Fragment,
    Compute,
};

enum class EPrimitiveType {
    TriangleList,
    TriangleStrip,
    LineList,
    QuadList,
    PointList,
};

enum class EPixelFormat {
    Unknown,
    R8G8B8A8_SRGB,
};

enum class ERasterizerFillMode {
    Point,
    Wireframe,
    Solid,
};

enum class ERasterizerCullMode {
    None,
    ClockWise,
    CounterClockWise,
};

enum class ERasterizerDepthClipMode {
    DepthClip,
    DepthClamp,
};

enum class ETextureCreateFlags {
    None = 0,
    RenderTargetable = BIT(0),
    ResolveTargetable = BIT(1),
    DepthStencilTargetable = BIT(2),
};
ENUM_CLASS_FLAGS(ETextureCreateFlags)

enum class ETextureDimension {
    Texture2D,
};

struct RHITextureCreateDesc {
    const std::string DebugName;
    Ref<Buffer> InitialData = nullptr;

    ETextureCreateFlags Flags = ETextureCreateFlags::None;
    ETextureDimension Dimension = ETextureDimension::Texture2D;

    glm::uvec2 Extent = {1, 1};
    uint32 Depth = 1;
    uint8 NumMips = 1;
    uint8 NumSamples = 0;
};

struct RasterizerCreateInfo {
    ERasterizerFillMode FillMode = ERasterizerFillMode::Solid;
    ERasterizerCullMode CullMode = ERasterizerCullMode::ClockWise;
    ERasterizerDepthClipMode ClipMode = ERasterizerDepthClipMode::DepthClip;
    float DepthBias = 0.0f;

    bool AllowMSAA = false;

    bool operator==(const RasterizerCreateInfo &) const = default;
};

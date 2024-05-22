#pragma once

#include "Engine/Misc/EnumFlags.hxx"
#include <glm/gtx/hash.hpp>

/// @brief The max amount of render target that may used used simultaneously
constexpr unsigned MaxRenderTargets = 8;

/// @brief Dimension of an RHI Texture
enum class EImageDimension {
    Texture2D,
};

/// @brief Format of an RHI Texture
enum class EImageFormat {
    D32_SFLOAT,    // <-- Note: may be not add depth format to this enum
    R8G8B8_SRGB,
    R8G8B8A8_SRGB,
    B8G8R8A8_SRGB,
};

/// The Type of the Vertex
enum class EVertexElementType {
    Float1,
    Float2,
    Float3,
    Float4,
    Uint1,
    Uint2,
    Uint3,
    Uint4,
    Int1,
    Int2,
    Int3,
    Int4,
};

/// @brief How the polygon should be rendered
enum class EPolygonMode {
    Fill,
    Line,
    Point,
};

/// @brief Configure the back-face culling
enum class ECullMode {
    None,
    Front,
    Back,
    FrontAndBack,
};

/// @brief The unwinding order of the meshes
enum class EFrontFace {
    CounterClockwise,
    Clockwise,
};

/// @brief What the texture will be used for
enum class ETextureUsageFlags {
    None = 0,
    RenderTargetable = BIT(0),
    ResolveTargetable = BIT(1),
    DepthStencilTargetable = BIT(2),

    SampleTargetable = BIT(3),
    TransferTargetable = BIT(4),
};
ENUM_CLASS_FLAGS(ETextureUsageFlags)

/// Action to take when a render target is set.
enum class ERenderTargetLoadAction : uint8 {
    /// Untouched contents of the render target are undefined. Any existing content is not preserved.
    NoAction,

    /// Existing contents are preserved.
    Load,

    /// The render target is cleared to the fast clear value specified on the resource.
    Clear,
};

/// Action to take when a render target is unset or at the end of a pass.
enum class ERenderTargetStoreAction : uint8 {
    /// Contents of the render target emitted during the pass are not stored back to memory.
    NoAction,

    /// Contents of the render target emitted during the pass are stored back to memory.
    Store,
};

uint32 GetSizeOfElementType(EVertexElementType Type);

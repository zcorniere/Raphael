#pragma once

#include "Engine/Misc/EnumFlags.hxx"

/// @brief Dimension of an RHI Texture
enum class EImageDimension : uint8
{
    Texture2D,
};

/// @brief Format of an RHI Texture
enum class EImageFormat : uint8
{
    D32_SFLOAT,
    R8G8B8_SRGB,
    R8G8B8A8_SRGB,
    B8G8R8A8_SRGB,
};

/// The Type of the Vertex
enum class EVertexElementType : uint8
{
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

/// @brief The input mode of the vertex
enum class EVertexInputMode : uint8
{
    PerVertex,
    PerInstance,
};

/// @brief How the polygon should be rendered
enum class EPolygonMode : uint8
{
    Fill,
    Line,
    Point,
};

/// @brief Configure the back-face culling
enum class ECullMode : uint8
{
    None,
    Front,
    Back,
    FrontAndBack,
};

/// @brief The unwinding order of the meshes
enum class EFrontFace : uint8
{
    CounterClockwise,
    Clockwise,
};

/// @brief What the texture will be used for
enum class ETextureUsageFlags : uint32
{
    None = 0,
    RenderTargetable = BIT(0),
    ResolveTargetable = BIT(1),
    DepthStencilTargetable = BIT(2),

    SampleTargetable = BIT(3),
    TransferTargetable = BIT(4),
};
ENUM_CLASS_FLAGS(ETextureUsageFlags)

/// Action to take when a render target is set.
enum class ERenderTargetLoadAction : uint8
{
    /// Untouched contents of the render target are undefined. Any existing content is not preserved.
    NoAction,

    /// Existing contents are preserved.
    Load,

    /// The render target is cleared to the fast clear value specified on the resource.
    Clear,
};

/// Action to take when a render target is unset or at the end of a pass.
enum class ERenderTargetStoreAction : uint8
{
    /// Contents of the render target emitted during the pass are not stored back to memory.
    NoAction,

    /// Contents of the render target emitted during the pass are stored back to memory.
    Store,
};

uint32 GetSizeOfElementType(EVertexElementType Type);

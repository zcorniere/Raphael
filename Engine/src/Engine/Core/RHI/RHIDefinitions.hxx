#pragma once

#include "Engine/Misc/EnumFlags.hxx"

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

class RHITexture;
class RHIGraphicsPipeline;

struct RHIRenderPassDescription {
    struct RenderingTarget {
        Ref<RHITexture> TargetTexture;
    };

    Array<RenderingTarget> ColorTarget;
    Array<RenderingTarget> ResolveTarget;
    RenderingTarget DepthTarget;
};

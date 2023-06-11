#pragma once

#include "Engine/Misc/EnumFlags.hxx"

enum class EImageDimension {
    Texture2D,
};

enum class EImageFormat {
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

enum class EPolygonMode {
    Fill,
    Line,
    Point,
};

enum class ECullMode {
    None,
    Front,
    Back,
    FrontAndBack,
};

enum class EFrontFace {
    CounterClockwise,
    Clockwise,
};

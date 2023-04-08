#pragma once

#include "Engine/Core/Buffer.hxx"
#include "Engine/Misc/EnumFlags.hxx"

enum class EImageDimension {
    Texture2D,
};

enum class EImageFormat {
    R8G8B8_SRGB,
    R8G8B8A8_RGBA,
};

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
ENUM_CLASS_FLAGS(EVertexElementType)

struct VertexElement {
    uint8 Offset;
    uint16 Stride;
    uint8 AttributeIndex;
    EVertexElementType VertexType;

    bool operator==(const VertexElement &) const = default;
};
using VertexElementList = std::vector<VertexElement>;

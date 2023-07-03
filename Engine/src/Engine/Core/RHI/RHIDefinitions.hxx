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
    struct RenderingTargetInfo {
        EImageFormat Format;
        bool operator==(const RenderingTargetInfo&) const = default;
    };

    Array<RenderingTargetInfo> ColorTarget;
    Array<RenderingTargetInfo> ResolveTarget;
    std::optional<RenderingTargetInfo> DepthTarget;

    glm::ivec2 Offset;
    glm::uvec2 Size;
    std::string Name;

    bool operator==(const RHIRenderPassDescription&) const = default;
};

class ResourceArray : public RObject
{
public:
    /// @return A pointer to the resource data.
    virtual const void* GetResourceData() const = 0;

    /// @return size of resource data allocation
    virtual uint32 GetResourceDataSize() const = 0;

    /// Called after the RHI has copied the resource data, and no longer needs the CPU's copy.
    virtual void Discard() = 0;
};

template <typename ElementType>
class TResourceArray : public Array<ElementType>, public ResourceArray
{
public:
    const void* GetResourceData() const
    {
        return this->Raw();
    }

    uint32 GetResourceDataSize() const
    {
        checkMsg(this->Size() > UINT32_MAX / sizeof(ElementType),
                 "Resource data size too large for uint32, will overflow. Calculate with larger data type or "
                 "use fewer elements. sizeof(ElementType): {:d}",
                 sizeof(ElementType));
        return this->Size() * sizeof(ElementType);
    }

    void Discard()
    {
        this->Empty();
    }
};

namespace std
{

template <>
struct hash<RHIRenderPassDescription> {
    size_t operator()(const RHIRenderPassDescription& Desc) const
    {
        size_t Result = 0;
        ::Raphael::HashCombine(Result, Desc.Offset);
        ::Raphael::HashCombine(Result, Desc.Size);
        for (const RHIRenderPassDescription::RenderingTargetInfo& Target: Desc.ColorTarget) {
            ::Raphael::HashCombine(Result, Target.Format);
        }
        for (const RHIRenderPassDescription::RenderingTargetInfo& Target: Desc.ResolveTarget) {
            ::Raphael::HashCombine(Result, Target.Format);
        }
        if (Desc.DepthTarget) {
            ::Raphael::HashCombine(Result, Desc.DepthTarget->Format);
        }
        return Result;
    }
};

}    // namespace std

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

class RHITexture;
class RHIGraphicsPipeline;

struct RHIRenderPassTarget {
    EImageFormat Format;
    ETextureUsageFlags Flags;

    bool operator==(const RHIRenderPassTarget&) const = default;
};

struct RHIRenderPassDescription {
    Array<RHIRenderPassTarget> ColorTarget = {};
    Array<RHIRenderPassTarget> ResolveTarget = {};
    std::optional<RHIRenderPassTarget> DepthTarget = std::nullopt;

    bool operator==(const RHIRenderPassDescription&) const = default;
};

struct RHIFramebufferDefinition {
    Array<Ref<RHITexture>> ColorTarget = {};
    Array<Ref<RHITexture>> ResolveTarget = {};
    Ref<RHITexture> DepthTarget = nullptr;

    glm::ivec2 Offset;
    glm::uvec2 Extent;

    bool operator==(const RHIFramebufferDefinition&) const = default;
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
    const void* GetResourceData() const override
    {
        return this->Raw();
    }

    uint32 GetResourceDataSize() const override
    {
        checkMsg(this->Size() > UINT32_MAX / sizeof(ElementType),
                 "Resource data size too large for uint32, will overflow. Calculate with larger data type or "
                 "use fewer elements. sizeof(ElementType): {:d}",
                 sizeof(ElementType));
        return this->Size() * sizeof(ElementType);
    }

    void Discard() override
    {
        this->Empty();
    }
};

namespace std
{

template <>
struct hash<RHIRenderPassDescription> {
    size_t operator()(const RHIRenderPassDescription& Desc) const;
};

template <>
struct hash<RHIFramebufferDefinition> {
    size_t operator()(const RHIFramebufferDefinition& Desc) const;
};

}    // namespace std

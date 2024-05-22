#pragma once

#include "Engine/Misc/EnumFlags.hxx"

#include "Engine/Containers/ResourceArray.hxx"
#include "Engine/Core/RHI/RHIResource.hxx"

enum class EBufferUsageFlags {
    None = 0,

    Static = BIT(0),
    Transient = BIT(1),

    KeepCPUAccessible = BIT(2),

    SourceCopy = BIT(3),
    DestinationCopy = BIT(4),

    DrawIndirect = BIT(5),

    VertexBuffer = BIT(6),
    IndexBuffer = BIT(7),
    StructuredBuffer = BIT(8),
};
ENUM_CLASS_FLAGS(EBufferUsageFlags);

struct RHIBufferDesc {
    /// Size in bytes of the buffer
    uint32 Size = 0;
    uint32 Stride = 0;
    EBufferUsageFlags Usage = EBufferUsageFlags::None;
    ResourceArrayInterface* ResourceArray = nullptr;
    std::string DebugName;
};

/// @brief Represent a Buffer used by the RHI
class RHIBuffer : public RHIResource
{
public:
    RHIBuffer(const RHIBufferDesc& InDescription): RHIResource(ERHIResourceType::Buffer), Description(InDescription)
    {
    }
    virtual ~RHIBuffer() = default;

    /// @return The number of bytes in the buffer.
    uint32 GetSize() const
    {
        return Description.Size;
    }

    /// @return The stride in bytes of the buffer.
    uint32 GetStride() const
    {
        return Description.Stride;
    }

    /// @return The usage flags used to create the buffer.
    EBufferUsageFlags GetUsage() const
    {
        return Description.Usage;
    }

protected:
    RHIBufferDesc Description;
};

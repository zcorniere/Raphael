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
    StorageBuffer = BIT(8),
    UniformBuffer = BIT(9),
};
ENUM_CLASS_FLAGS(EBufferUsageFlags);

struct FRHIBufferDesc {
    /// Size in bytes of the buffer
    uint32 Size = 0;
    uint32 Stride = 0;
    EBufferUsageFlags Usage = EBufferUsageFlags::None;
    IResourceArrayInterface* ResourceArray = nullptr;
    std::string DebugName;
};

/// @brief Represent a Buffer used by the RHI
class RRHIBuffer : public RRHIResource
{
    RTTI_DECLARE_TYPEINFO(RRHIBuffer, RRHIResource);

public:
    RRHIBuffer(const FRHIBufferDesc& InDescription): RRHIResource(ERHIResourceType::Buffer), Description(InDescription)
    {
    }
    virtual ~RRHIBuffer() = default;

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
    FRHIBufferDesc Description;
};

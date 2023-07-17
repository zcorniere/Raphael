#pragma once

#include "Engine/Core/RHI/RHIDefinitions.hxx"
#include "Engine/Core/RHI/RHIResource.hxx"
#include "Engine/Misc/EnumFlags.hxx"

#include <glm/vec3.hpp>

enum class EBufferUsageFlags {
    None = 0,

    SourceCopy = BIT(0),
    DestinationCopy = BIT(1),

    DrawIndirect = BIT(2),

    VertexBuffer = BIT(3),
    IndexBuffer = BIT(4),
    StructuredBuffer = BIT(5),
};

/// @brief Represent a Buffer used by the RHI
class RHIBuffer : public RHIResource
{
public:
    RHIBuffer(): RHIResource(RHIResourceType::Buffer), Size(0), Stride(0), Usage(EBufferUsageFlags::None)
    {
    }

    RHIBuffer(EBufferUsageFlags InUsage, uint32 InSize, uint32 InStride)
        : RHIResource(RHIResourceType::Buffer), Size(InSize), Stride(InStride), Usage(InUsage)
    {
    }

    /// @return The number of bytes in the buffer.
    uint32 GetSize() const
    {
        return Size;
    }

    /// @return The stride in bytes of the buffer.
    uint32 GetStride() const
    {
        return Stride;
    }

    /// @return The usage flags used to create the buffer.
    EBufferUsageFlags GetUsage() const
    {
        return Usage;
    }

private:
    uint32 Size;
    uint32 Stride;
    EBufferUsageFlags Usage;
};

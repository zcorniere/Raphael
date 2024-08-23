#pragma once

#include "Engine/Core/RHI/RHIResource.hxx"

class RHITexture;

/// @brief Represent a viewport used by the RHI to be drawn on
class RHIViewport : public RHIResource
{
    RTTI_DECLARE_TYPEINFO(RHIViewport, RHIResource);

public:
    RHIViewport(): RHIResource(ERHIResourceType::Viewport)
    {
    }
    virtual ~RHIViewport() = default;

    /// @brief The current viewport will be resized
    virtual void ResizeViewport(uint32 Width, uint32 Height) = 0;

    virtual Ref<RHITexture> GetBackbuffer() const = 0;

    virtual UVector2 GetSize() const = 0;
};

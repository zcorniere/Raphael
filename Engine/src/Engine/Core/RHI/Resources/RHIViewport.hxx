#pragma once

#include "Engine/Core/RHI/RHIResource.hxx"

class RRHITexture;

/// @brief Represent a viewport used by the RHI to be drawn on
class RRHIViewport : public RRHIResource
{
    RTTI_DECLARE_TYPEINFO(RRHIViewport, RRHIResource);

public:
    RRHIViewport(): RRHIResource(ERHIResourceType::Viewport)
    {
    }
    virtual ~RRHIViewport() = default;

    /// @brief The current viewport will be resized
    virtual void ResizeViewport(uint32 Width, uint32 Height) = 0;

    virtual Ref<RRHITexture> GetBackbuffer() const = 0;
    virtual Ref<RRHITexture> GetDepthBuffer() const = 0;

    virtual UVector2 GetSize() const = 0;
    virtual float GetAspectRatio() const
    {
        return static_cast<float>(GetSize().x) / static_cast<float>(GetSize().y);
    }
};

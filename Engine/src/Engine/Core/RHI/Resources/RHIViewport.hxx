#pragma once

#include "Engine/Core/RHI/RHIResource.hxx"

class RHITexture;

/// @brief Represent a viewport used by the RHI to be drawn on
class RHIViewport : public RHIResource
{
public:
    RHIViewport(): RHIResource(RHIResourceType::Viewport)
    {
    }
    virtual ~RHIViewport() = default;

    /// @brief Mark the current viewport as the one to draw on
    ///
    /// Will be queued in the RHI command, and be executed later
    void BeginDrawViewport();
    /// @brief The current viewport can be rendered
    ///
    /// Will be queued in the RHI command, and be executed later
    void EndDrawViewport();

    /// @brief The current viewport will be resized
    virtual void ResizeViewport(uint32 Width, uint32 Height) = 0;

    virtual Ref<RHITexture> GetBackbuffer() const = 0;

    virtual glm::uvec2 GetSize() const = 0;
};

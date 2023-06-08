#pragma once

#include "Engine/Core/RHI/RHIResource.hxx"

/// @brief Represent a viewport used by the RHI to be drawn on
class RHIViewport : public RHIResource
{
public:
    RHIViewport(): RHIResource(RHIResourceType::Viewport)
    {
    }

    /// @brief Mark the current viewport as the one to draw on
    ///
    /// Will be queued in the RHI command, and be executed later
    void BeginDrawViewport();
    /// @brief The current viewport can be rendered
    ///
    /// Will be queued in the RHI command, and be executed later
    void EndDrawViewport();

private:
    virtual void RT_BeginDrawViewport() = 0;
    virtual void RT_EndDrawViewport() = 0;
};

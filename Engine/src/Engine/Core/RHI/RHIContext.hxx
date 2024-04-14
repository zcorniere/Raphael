#pragma once

#include "Engine/Core/RHI/Resources/RHIViewport.hxx"

class RHIContext
{
public:
    virtual ~RHIContext() = default;

    /// @brief Mark the beginning of a new frame
    virtual void BeginFrame() = 0;
    /// @brief Mark the end of the current frame
    virtual void EndFrame() = 0;

    /// @brief Indicate the RHI that we are starting drawing in the given viewport
    virtual void RHIBeginDrawingViewport(RHIViewport* const Viewport) = 0;
    /// @brief Indicate the RHI that we are done drawing in the given viewport
    virtual void RHIEndDrawningViewport(RHIViewport* const Viewport) = 0;
    /// @brief Resize the given viewport
    virtual void RHIResizeViewport(RHIViewport* const Viewport, uint32_t Width, uint32_t Height) = 0;
};

#pragma once

#include "Engine/Core/RHI/RHIResource.hxx"

class RHIViewport : public RHIResource
{
public:
    RHIViewport(): RHIResource(RHIResourceType::Viewport) {}

    void BeginDrawViewport();
    void EndDrawViewport();

private:
    virtual void RT_BeginDrawViewport() = 0;
    virtual void RT_EndDrawViewport() = 0;
};

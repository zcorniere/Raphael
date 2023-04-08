#pragma once

#include "Engine/Renderer/RHI/RHIResource.hxx"

class RHIViewport : public RHIResource
{
public:
    RHIViewport(): RHIResource(RHIResourceType::Viewport) {}
};

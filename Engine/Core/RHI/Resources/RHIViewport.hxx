#pragma once

#include "Engine/Core/RHI/RHIResource.hxx"

class RHIViewport : public RHIResource
{
public:
    RHIViewport(): RHIResource(RHIResourceType::Viewport) {}
};

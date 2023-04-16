#pragma once

#include "Engine/Core/RHI/RHIDefinitions.hxx"
#include "Engine/Core/RHI/RHIResource.hxx"

struct RHIRenderPass : public RHIResource {
public:
    RHIRenderPass(): RHIResource(RHIResourceType::RenderPass) {}
};

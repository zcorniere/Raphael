#pragma once

#include "Engine/Renderer/RHI/RHIDefinitions.hxx"
#include "Engine/Renderer/RHI/RHIResource.hxx"

struct RHIRenderPass : public RHIResource {
public:
    RHIRenderPass(): RHIResource(RHIResourceType::RenderPass) {}
};

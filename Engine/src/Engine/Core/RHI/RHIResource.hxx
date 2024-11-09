#pragma once

/// The type of RHI resource
enum class ERHIResourceType {
    None = 0,

    Texture,
    Buffer,
    Shader,
    Viewport,
    GraphicsPipeline,

    MAX_VALUE,
};

/// Represent and abstract above RHI resources
class RRHIResource : public RObject
{
    RTTI_DECLARE_TYPEINFO(RRHIResource, RObject);

public:
    RRHIResource() = delete;
    explicit RRHIResource(ERHIResourceType InResourceType): ResourceType(InResourceType)
    {
    }

    virtual ~RRHIResource() = default;

protected:
    const ERHIResourceType ResourceType;
};

// IWYU pragma: begin_exports
#include "Engine/Core/RHI/Resources/RHIBuffer.hxx"
#include "Engine/Core/RHI/Resources/RHIGraphicsPipeline.hxx"
#include "Engine/Core/RHI/Resources/RHIShader.hxx"
#include "Engine/Core/RHI/Resources/RHITexture.hxx"
#include "Engine/Core/RHI/Resources/RHIViewport.hxx"
// IWYU pragma: end_exports

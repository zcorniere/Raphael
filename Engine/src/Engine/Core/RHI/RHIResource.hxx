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
class RHIResource : public RObject
{
    RTTI_DECLARE_TYPEINFO(RHIResource, RObject);

public:
    RHIResource() = delete;
    explicit RHIResource(ERHIResourceType InResourceType): ResourceType(InResourceType)
    {
    }

    virtual ~RHIResource() = default;

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

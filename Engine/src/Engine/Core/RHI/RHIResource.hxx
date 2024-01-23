#pragma once

/// The type of THI resource
enum class RHIResourceType : uint8 {
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
public:
    RHIResource() = delete;
    explicit RHIResource(RHIResourceType InResourceType): ResourceType(InResourceType)
    {
    }

    virtual ~RHIResource() = default;

protected:
    const RHIResourceType ResourceType;
};

#include "Engine/Core/RHI/Resources/RHIBuffer.hxx"              // IWYU pragma: export
#include "Engine/Core/RHI/Resources/RHIGraphicsPipeline.hxx"    // IWYU pragma: export
#include "Engine/Core/RHI/Resources/RHIShader.hxx"              // IWYU pragma: export
#include "Engine/Core/RHI/Resources/RHITexture.hxx"             // IWYU pragma: export
#include "Engine/Core/RHI/Resources/RHIViewport.hxx"            // IWYU pragma: export

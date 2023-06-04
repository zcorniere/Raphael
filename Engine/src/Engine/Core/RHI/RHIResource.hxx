#pragma once

#include <glm/vec3.hpp>
#include <magic_enum.hpp>

/// The type of THI resource
enum class RHIResourceType : uint8 {
    None = 0,

    Texture,
    Shader,
    Viewport,

    MAX_VALUE,
};

/// Represent and abstract abore RHI ressources
class RHIResource : public RObject
{
public:
    RHIResource(RHIResourceType InResourceType): ResourceType(InResourceType)
    {
    }

    virtual ~RHIResource()
    {
    }

protected:
    const RHIResourceType ResourceType;
};

#include "Engine/Core/RHI/Resources/RHIShader.hxx"
#include "Engine/Core/RHI/Resources/RHITexture.hxx"
#include "Engine/Core/RHI/Resources/RHIViewport.hxx"

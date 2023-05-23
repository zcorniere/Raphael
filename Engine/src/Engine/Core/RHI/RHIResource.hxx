#pragma once

#include <glm/vec3.hpp>
#include <magic_enum.hpp>

enum class RHIResourceType : uint8 {
    None = 0,

    Texture,
    Shader,
    Viewport,

    MAX_VALUE,
};

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

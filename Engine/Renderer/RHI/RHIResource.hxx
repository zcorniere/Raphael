#pragma once

#include "Engine/Core/Buffer.hxx"

#include <glm/vec3.hpp>
#include <magic_enum.hpp>

enum class RHIResourceType : uint8 {
    None = 0,

    Texture,
    Shader,
    Viewport,

    Framebuffer,
    RenderPass,

    MAX_VALUE,
};

class RHIResource : public RObject
{
public:
    RHIResource(RHIResourceType InResourceType, std::string_view InName = ""): ResourceType(InResourceType)
    {
        SetName(InName);
    }

    virtual ~RHIResource()
    {
    }

protected:
    const RHIResourceType ResourceType;
};

#include "Engine/Renderer/RHI/Resources/RHIFramebuffer.hxx"
#include "Engine/Renderer/RHI/Resources/RHIRenderPass.hxx"
#include "Engine/Renderer/RHI/Resources/RHIShader.hxx"
#include "Engine/Renderer/RHI/Resources/RHITexture.hxx"
#include "Engine/Renderer/RHI/Resources/RHIViewport.hxx"

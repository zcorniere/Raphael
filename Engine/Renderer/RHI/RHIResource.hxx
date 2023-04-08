#pragma once

#include "Engine/Core/Buffer.hxx"

#include <glm/vec3.hpp>
#include <magic_enum.hpp>

enum class RHIResourceType : uint8 {
    None,

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

    virtual void SetName(std::string_view InName) override
    {
        // Prefix the resource name with the ResourceType
        std::string ResourceTypeName(magic_enum::enum_name(ResourceType));
        RObject::SetName(ResourceTypeName + "(" + std::string(InName) + ")");
    }

private:
    const RHIResourceType ResourceType;
};

#include "Engine/Renderer/RHI/Resources/RHIFramebuffer.hxx"
#include "Engine/Renderer/RHI/Resources/RHIRenderPass.hxx"
#include "Engine/Renderer/RHI/Resources/RHIShader.hxx"
#include "Engine/Renderer/RHI/Resources/RHITexture.hxx"
#include "Engine/Renderer/RHI/Resources/RHIViewport.hxx"

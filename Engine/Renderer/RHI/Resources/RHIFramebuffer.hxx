#pragma once

#include "Engine/Renderer/RHI/RHIDefinitions.hxx"
#include "Engine/Renderer/RHI/RHIResource.hxx"

enum class FramebufferBlendMode {
    None = 0,
    OneZero,
    SrcAlphaOneMinusSrcAlpha,
    Additive,
    Zero_SrcColor,
};

struct FramebufferTextureSpecification {
    EImageFormat Format;
    FramebufferBlendMode BlendMode = FramebufferBlendMode::SrcAlphaOneMinusSrcAlpha;
};
using FramebufferAttachmentSpecification = std::vector<FramebufferTextureSpecification>;

struct FramebufferSpecification {
    uint32 Width = 0;
    uint32 Height = 0;
    glm::vec4 ClearColor = {0.0f, 0.0f, 0.0f, 1.0f};

    FramebufferAttachmentSpecification Attachments;
};

class RHIFramebuffer : public RHIResource
{
public:
    RHIFramebuffer(const FramebufferSpecification &InSpecification)
        : RHIResource(RHIResourceType::Framebuffer), Specification(InSpecification)
    {
    }

    const FramebufferSpecification &GetSpecification() const { return Specification; }

private:
    FramebufferSpecification Specification;
};

#pragma once

/// The type of RHI resource
enum class RHIResourceType : uint8 {
    None = 0,

    Texture,
    Buffer,
    Shader,
    Viewport,
    GraphicsPipeline,

    MAX_VALUE,
};

/// Represent and abstract type above RHI ressources
class RHIResource : public RefCounted, public NamedClassWithTypeName<RHIResource>
{
public:
    RHIResource() = delete;
    RHIResource(RHIResourceType InResourceType): ResourceType(InResourceType)
    {
    }

    virtual ~RHIResource()
    {
    }

private:
    // RefCounted interface
    void Destroy() const override
    {
        delete this;
    }

protected:
    mutable std::atomic<uint32> m_RefCount = 1;
    const RHIResourceType ResourceType;
};

class RHIBuffer;
class RHITexture;
class RHIShader;
class RHIViewport;
class RHIGraphicsPipeline;

using RHIBufferRef = TRefCountPtr<RHIBuffer>;
using RHITextureRef = TRefCountPtr<RHITexture>;
using RHIShaderRef = TRefCountPtr<RHIShader>;
using RHIViewportRef = TRefCountPtr<RHIViewport>;
using RHIGraphicsPipelineRef = TRefCountPtr<RHIGraphicsPipeline>;

#include "Engine/Core/RHI/Resources/RHIBuffer.hxx"
#include "Engine/Core/RHI/Resources/RHIGraphicsPipeline.hxx"
#include "Engine/Core/RHI/Resources/RHIShader.hxx"
#include "Engine/Core/RHI/Resources/RHITexture.hxx"
#include "Engine/Core/RHI/Resources/RHIViewport.hxx"

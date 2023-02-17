#pragma once

#include "Engine/Core/Buffer.hxx"
#include "Engine/Renderer/RHI/RHIDefinitions.hxx"

#include <glm/vec3.hpp>

class RHIResource : public RObject
{
public:
    RHIResource(RHIResourceType InResourceType, std::string InName = ""): Name(InName), ResourceType(InResourceType)
    {
    }

    virtual ~RHIResource()
    {
    }

    void SetName(std::string_view InName)
    {
        Name = InName;
    }

    const std::string &GetName() const
    {
        return Name;
    }

private:
    std::string Name;
    const RHIResourceType ResourceType;
};

class RHITexture : public RHIResource
{
public:
    RHITexture(const RHITextureCreateDesc &InDesc): RHIResource(RHIResourceType::Texture), Description(InDesc)
    {
    }

    virtual const RHITextureCreateDesc &GetDescription() const
    {
        return Description;
    }

    // RHI specific function
    virtual void *GetNativeResource() const
    {
        return nullptr;
    }

    glm::ivec3 GetMipDimensions(uint8 MipIndex) const
    {
        const RHITextureCreateDesc &Desc = GetDescription();
        return glm::ivec3(std::max(Desc.Extent.x >> MipIndex, 1u), std::max(Desc.Extent.y >> MipIndex, 1u),
                          std::max(Desc.Depth >> MipIndex, 1u));
    }

private:
    const RHITextureCreateDesc Description;
};

class RHIViewport : public RHIResource
{
public:
    RHIViewport(): RHIResource(RHIResourceType::Viewport)
    {
    }
};

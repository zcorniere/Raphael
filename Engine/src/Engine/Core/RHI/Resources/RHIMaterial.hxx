#pragma once

#include "Engine/Core/RHI/RHIResource.hxx"

class RRHIMaterial : public RRHIResource
{
    RTTI_DECLARE_TYPEINFO(RRHIMaterial, RRHIResource)

public:
    explicit RRHIMaterial(): Super(ERHIResourceType::Material)
    {
    }
    virtual ~RRHIMaterial() = default;

    virtual void Bake() = 0;
    virtual bool WasBaked() const = 0;

    virtual void SetInput(std::string_view Name, const Ref<RRHIBuffer>& Buffer) = 0;
    virtual void SetInput(std::string_view Name, const Ref<RRHITexture>& Texture) = 0;
};

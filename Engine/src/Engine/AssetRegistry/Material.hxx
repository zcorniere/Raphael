#pragma once

#include "Engine/Core/RHI/Resources/RHIGraphicsPipeline.hxx"

class RMaterial : public RObject
{
    RTTI_DECLARE_TYPEINFO(RMaterial, RObject)
public:
    RMaterial(FRHIGraphicsPipelineSpecification Pipeline);
    ~RMaterial();

    void SetName(std::string_view Name) override;

    void Bind();
    void Unbind();

private:
    FRHIGraphicsPipelineSpecification Pipeline;
    Ref<RRHIGraphicsPipeline> GraphicsPipeline;
};

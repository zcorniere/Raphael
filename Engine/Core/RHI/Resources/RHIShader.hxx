#pragma once

#include "Engine/Core/RHI/RHIResource.hxx"

enum class RHIShaderType : uint8 {
    Vertex,
    Fragment,
    Compute,
};

class RHIShader : public RHIResource
{
public:
    RHIShader(RHIShaderType Type): RHIResource(RHIResourceType::Shader), ShaderType(Type) {}

    RHIShaderType GetShaderType() const { return ShaderType; }

private:
    const RHIShaderType ShaderType;
};

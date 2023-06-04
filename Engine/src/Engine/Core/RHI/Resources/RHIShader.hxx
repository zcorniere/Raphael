#pragma once

#include "Engine/Core/RHI/RHIResource.hxx"

/// @brief The type of the shader
enum class RHIShaderType : uint8 {
    Vertex,
    Fragment,
    Compute,
};

/// @brief Represent a shader used by the RHI
class RHIShader : public RHIResource
{
public:
    RHIShader(RHIShaderType Type): RHIResource(RHIResourceType::Shader), ShaderType(Type)
    {
    }

    /// @brief Return the type of the shader
    RHIShaderType GetShaderType() const
    {
        return ShaderType;
    }

private:
    const RHIShaderType ShaderType;
};

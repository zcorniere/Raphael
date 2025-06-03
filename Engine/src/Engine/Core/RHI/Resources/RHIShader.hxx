#pragma once

#include "Engine/Core/RHI/RHIResource.hxx"

/// @brief The type of the shader
enum class ERHIShaderType : uint8
{
    Vertex = 0,
    Fragment = 1,

    Compute,
};

/// @brief Represent a shader used by the RHI
class RRHIShader : public RRHIResource
{
    RTTI_DECLARE_TYPEINFO(RRHIShader, RRHIResource);

public:
    explicit RRHIShader(ERHIShaderType Type): RRHIResource(ERHIResourceType::Shader), ShaderType(Type)
    {
    }

    virtual ~RRHIShader() = default;

    /// @brief Return the type of the shader
    ERHIShaderType GetShaderType() const
    {
        return ShaderType;
    }

private:
    const ERHIShaderType ShaderType;
};

#pragma once

#include "Engine/Core/RHI/RHIResource.hxx"

/// @brief The type of the shader
enum class ERHIShaderType : uint8 {
    Vertex = 0,
    Pixel = 1,

    Compute,
};

/// @brief Represent a shader used by the RHI
class RHIShader : public RHIResource
{
    RTTI_DECLARE_TYPEINFO(RHIShader, RHIResource);

public:
    explicit RHIShader(ERHIShaderType Type): RHIResource(ERHIResourceType::Shader), ShaderType(Type)
    {
    }

    virtual ~RHIShader() = default;

    /// @brief Return the type of the shader
    ERHIShaderType GetShaderType() const
    {
        return ShaderType;
    }

private:
    const ERHIShaderType ShaderType;
};

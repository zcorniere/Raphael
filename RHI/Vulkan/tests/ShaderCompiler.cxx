#include "Engine/Raphael.hxx"

#include "Engine/Core/Log.hxx"

#include "Engine/Serialization/FileStream.hxx"
#include "VulkanRHI/Resources/VulkanShader.hxx"
#include "VulkanRHI/VulkanShaderCompiler.hxx"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

std::filesystem::path GetCurrentFilePath()
{
    std::source_location Location = std::source_location::current();
    std::filesystem::path File(Location.file_name());

    return File.parent_path();
}

static void CheckReflection(const VulkanRHI::VulkanShader::ReflectionData& ExpectedReflection,
                            const VulkanRHI::VulkanShader::ReflectionData& GotReflection)
{
    REQUIRE(GotReflection.PushConstants.Size() == ExpectedReflection.PushConstants.Size());
    for (unsigned i = 0; i < GotReflection.PushConstants.Size(); i++) {
        CHECK(GotReflection.PushConstants[i] == ExpectedReflection.PushConstants[i]);
    }

    REQUIRE(GotReflection.StageInput.Size() == ExpectedReflection.StageInput.Size());
    for (unsigned i = 0; i < GotReflection.StageInput.Size(); i++) {
        CHECK(GotReflection.StageInput[i] == ExpectedReflection.StageInput[i]);
    }

    REQUIRE(GotReflection.StageOutput.Size() == ExpectedReflection.StageOutput.Size());
    for (unsigned i = 0; i < GotReflection.StageOutput.Size(); i++) {
        CHECK(GotReflection.StageOutput[i] == ExpectedReflection.StageOutput[i]);
    }
    CHECK(GotReflection == ExpectedReflection);
}

TEST_CASE("Vulkan Shader Compiler: Simple Compilation")
{
    using namespace VulkanRHI;
    ::Log::Init();

    std::filesystem::path SimpleShaderPath = GetCurrentFilePath() / "test_shaders/SimpleShader.vert";
    VulkanShaderCompiler Compiler;
    Compiler.SetOptimizationLevel(VulkanShaderCompiler::OptimizationLevel::None);

    Ref<VulkanShader> ShaderResult = Compiler.Get(SimpleShaderPath);
    REQUIRE(ShaderResult);

    SECTION("Test shader Cache")
    {
        Ref<VulkanShader> CachedResult = Compiler.Get(SimpleShaderPath);

        CHECK(ShaderResult == CachedResult);
    }

    CHECK(ShaderResult->GetShaderType() == ERHIShaderType::Vertex);

    const VulkanShader::ReflectionData ExpectedReflection{
        .StageInput =
            {
                {
                    .Name = "inVertex",
                    .Type = EVertexElementType::Float4,
                    .Location = 0,
                },
            },
        .StageOutput =
            {
                {
                    .Name = "outVertexPos",
                    .Type = EVertexElementType::Float4,
                    .Location = 0,
                },
                {
                    .Name = "outUV",
                    .Type = EVertexElementType::Uint2,
                    .Location = 1,
                },
            },
        .PushConstants = {},
    };
    const VulkanShader::ReflectionData& GotReflection = ShaderResult->GetReflectionData();

    CheckReflection(ExpectedReflection, GotReflection);

    ::Log::Shutdown();
}

BEGIN_SHADER_PARAMETER_STRUCT(PointLightStruct)
SHADER_PARAMETER(glm::vec4, position)
SHADER_PARAMETER(glm::vec4, color)
SHADER_PARAMETER(float, intensity)
SHADER_PARAMETER(float, falloff)
END_SHADER_PARAMETER_STRUCT();

BEGIN_SHADER_PARAMETER_STRUCT(DirectionalLight)
SHADER_PARAMETER(glm::vec4, orientation)
SHADER_PARAMETER(glm::vec4, color)
SHADER_PARAMETER(float, intensity)
END_SHADER_PARAMETER_STRUCT();

BEGIN_SHADER_PARAMETER_STRUCT(SpotLight)
SHADER_PARAMETER(glm::vec4, position)
SHADER_PARAMETER(glm::vec4, direction)
SHADER_PARAMETER(glm::vec4, color)
SHADER_PARAMETER(float, cutOff)
SHADER_PARAMETER(float, outerCutOff)
SHADER_PARAMETER(float, intensity)
END_SHADER_PARAMETER_STRUCT();

BEGIN_SHADER_PARAMETER_STRUCT(MaterialStruct)
SHADER_PARAMETER(float, alphaCutOff)
SHADER_PARAMETER(float, metallic)
SHADER_PARAMETER(float, roughness)
SHADER_PARAMETER(glm::vec4, baseColor)
SHADER_PARAMETER(glm::vec4, baseColorFactor)
SHADER_PARAMETER(glm::vec4, emissiveFactor)
SHADER_PARAMETER(int, baseColorTexture)
SHADER_PARAMETER(int, metallicRoughnessTexture)
SHADER_PARAMETER(int, normalTexture)
SHADER_PARAMETER(int, occlusionTexture)
SHADER_PARAMETER(int, emissiveTexture)
SHADER_PARAMETER(int, specularGlossinessTexture)
SHADER_PARAMETER(int, diffuseTexture)
END_SHADER_PARAMETER_STRUCT();

BEGIN_SHADER_PARAMETER_STRUCT(PushConstantStruct)
SHADER_PARAMETER(int, pointLightCount)
SHADER_PARAMETER(int, directLightCount)
SHADER_PARAMETER(int, spotLightCount)
SHADER_PARAMETER(glm::vec3, position)
END_SHADER_PARAMETER_STRUCT();

TEST_CASE("Vulkan Shader Compiler: Complex Compilation")
{
    using namespace VulkanRHI;
    ::Log::Init();

    std::filesystem::path SimpleShaderPath = GetCurrentFilePath() / "test_shaders/TestComplex.frag";
    VulkanShaderCompiler Compiler;
    Compiler.SetOptimizationLevel(VulkanRHI::VulkanShaderCompiler::OptimizationLevel::None);

    Ref<VulkanShader> ShaderResult = Compiler.Get(SimpleShaderPath);
    REQUIRE(ShaderResult);

    CHECK(ShaderResult->GetShaderType() == ERHIShaderType::Pixel);

    const uint32 ExpectedPushConstantOffset = 64;
    const ShaderResource::PushConstantRange ExpectedPushConstant{
        .Offset = ExpectedPushConstantOffset,
        .Size = ExpectedPushConstantOffset + sizeof(PushConstantStruct),
        .Parameter =
            {
                .Name = "push",
                .Type = EShaderBufferType::Struct,
                .Size = ExpectedPushConstantOffset + sizeof(PushConstantStruct),
                .Offset = ExpectedPushConstantOffset,
                .Columns = 1,
                .Rows = 1,
                .Members =
                    {
                        {
                            .Name = "push",
                            .Type = EShaderBufferType::Struct,
                            .Size = sizeof(PushConstantStruct),
                            .Offset = ExpectedPushConstantOffset,
                            .Columns = 1,
                            .Rows = 1,
                            .Members =
                                {
                                    {
                                        .Name = "pointLightCount",
                                        .Type = EShaderBufferType::Uint32,
                                        .Size = 4,
                                        .Offset = 0,
                                        .Columns = 1,
                                        .Rows = 1,
                                    },
                                    {
                                        .Name = "directLightCount",
                                        .Type = EShaderBufferType::Uint32,
                                        .Size = 4,
                                        .Offset = 4,
                                        .Columns = 1,
                                        .Rows = 1,
                                    },
                                    {
                                        .Name = "spotLightCount",
                                        .Type = EShaderBufferType::Uint32,
                                        .Size = 4,
                                        .Offset = 8,
                                        .Columns = 1,
                                        .Rows = 1,
                                    },
                                    {
                                        .Name = "position",
                                        .Type = EShaderBufferType::Float,
                                        .Size = 4,
                                        .Offset = 16,
                                        .Columns = 1,
                                        .Rows = 4,
                                    },
                                },
                        },
                    },
            },
    };

    SECTION("Test Serialization")
    {

        Serialization::FileStreamWriter Writer("test.txt");
        Writer.WriteObject(ExpectedPushConstant);
        Writer.Flush();

        Serialization::FileStreamReader Reader("test.txt");

        CHECK(Reader);
        CHECK(Reader.IsGood());

        ShaderResource::PushConstantRange GotPushConstant;
        Reader.ReadObject(GotPushConstant);

        CHECK(GotPushConstant == ExpectedPushConstant);
    }

    const VulkanShader::ReflectionData
        ExpectedReflection{
            .StageInput =
                {
                    {
                        .Name = "fragPosition",
                        .Type = EVertexElementType::Float3,
                        .Location = 0,
                    },
                    {
                        .Name = "fragNormal",
                        .Type = EVertexElementType::Float3,
                        .Location = 1,
                    },
                    {
                        .Name = "fragTextCoords",
                        .Type = EVertexElementType::Float2,
                        .Location = 2,
                    },
                    {
                        .Name = "fragColor",
                        .Type = EVertexElementType::Float3,
                        .Location = 3,
                    },
                    {
                        .Name = "fragTangent",
                        .Type = EVertexElementType::Float4,
                        .Location = 4,
                    },
                    {
                        .Name = "materialIndex",
                        .Type = EVertexElementType::Uint1,
                        .Location = 5,
                    },
                },
            .StageOutput =
                {
                    {
                        .Name = "outColor",
                        .Type = EVertexElementType::Float4,
                        .Location = 0,
                    },
                },
            .PushConstants =
                {
                    ExpectedPushConstant,
                },
            .StorageBuffers =
                {
                    ShaderResource::StorageBuffer{
                        .Set = 2,
                        .Binding = 1,
                        .Parameter =
                            {
                                ShaderParameter{
                                    .Name = "ObjectMaterials",
                                    .Type = EShaderBufferType::Struct,
                                    .Size = 0,
                                    .Offset = 0,
                                    .Columns = 1,
                                    .Rows = 1,
                                    .Members =
                                        {
                                            {
                                                .Name = "materials",
                                                .Type = EShaderBufferType::Struct,
                                                .Size = 92,
                                                .Offset = 0,
                                                .Columns = 1,
                                                .Rows = 1,
                                                .Members =
                                                    {
                                                        {
                                                            .Name = "alphaCutOff",
                                                            .Type = EShaderBufferType::Float,
                                                            .Size = 4,
                                                            .Offset = 0,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "metallic",
                                                            .Type = EShaderBufferType::Float,
                                                            .Size = 4,
                                                            .Offset = 4,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "roughness",
                                                            .Type = EShaderBufferType::Float,
                                                            .Size = 4,
                                                            .Offset = 8,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "baseColor",
                                                            .Type = EShaderBufferType::Float,
                                                            .Size = 4,
                                                            .Offset = 16,
                                                            .Columns = 1,
                                                            .Rows = 4,

                                                        },
                                                        {
                                                            .Name = "baseColorFactor",
                                                            .Type = EShaderBufferType::Float,
                                                            .Size = 4,
                                                            .Offset = 32,
                                                            .Columns = 1,
                                                            .Rows = 4,

                                                        },
                                                        {
                                                            .Name = "emissiveFactor",
                                                            .Type = EShaderBufferType::Float,
                                                            .Size = 4,
                                                            .Offset = 48,
                                                            .Columns = 1,
                                                            .Rows = 4,

                                                        },
                                                        {
                                                            .Name = "baseColorTexture",
                                                            .Type = EShaderBufferType::Int32,
                                                            .Size = 4,
                                                            .Offset = 64,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "metallicRoughnessTexture",
                                                            .Type = EShaderBufferType::Int32,
                                                            .Size = 4,
                                                            .Offset = 68,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "normalTexture",
                                                            .Type = EShaderBufferType::Int32,
                                                            .Size = 4,
                                                            .Offset = 72,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "occlusionTexture",
                                                            .Type = EShaderBufferType::Int32,
                                                            .Size = 4,
                                                            .Offset = 76,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "emissiveTexture",
                                                            .Type = EShaderBufferType::Int32,
                                                            .Size = 4,
                                                            .Offset = 80,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "specularGlossinessTexture",
                                                            .Type = EShaderBufferType::Int32,
                                                            .Size = 4,
                                                            .Offset = 84,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "diffuseTexture",
                                                            .Type = EShaderBufferType::Int32,
                                                            .Size = 4,
                                                            .Offset = 88,
                                                            .Columns = 1,
                                                            .Rows = 1,
                                                        },
                                                    },
                                            },
                                        },
                                },
                            },
                    },
                    ShaderResource::StorageBuffer{
                        .Set = 1,
                        .Binding = 0,
                        .Parameter =
                            ShaderParameter{
                                .Name = "DirectLight",
                                .Type = EShaderBufferType::Struct,
                                .Size = 0,
                                .Offset = 0,
                                .Columns = 1,
                                .Rows = 1,
                                .Members =
                                    {
                                        {
                                            .Name = "directionalLightArray",
                                            .Type = EShaderBufferType::Struct,
                                            .Size = 36,
                                            .Offset = 0,
                                            .Columns = 1,
                                            .Rows = 1,
                                            .Members =
                                                {
                                                    {
                                                        .Name = "orientation",
                                                        .Type = EShaderBufferType::Float,
                                                        .Size = 4,
                                                        .Offset = 0,
                                                        .Columns = 1,
                                                        .Rows = 4,

                                                    },
                                                    {
                                                        .Name = "color",
                                                        .Type = EShaderBufferType::Float,
                                                        .Size = 4,
                                                        .Offset = 16,
                                                        .Columns = 1,
                                                        .Rows = 4,

                                                    },
                                                    {
                                                        .Name = "intensity",
                                                        .Type = EShaderBufferType::Float,
                                                        .Size = 4,
                                                        .Offset = 32,
                                                        .Columns = 1,
                                                        .Rows = 1,
                                                    },
                                                },
                                        },
                                    },
                            },
                    },

                    ShaderResource::StorageBuffer{
                        .Set = 1,
                        .Binding = 1,
                        .Parameter =
                            {
                                ShaderParameter{
                                    .Name = "SpoLight",
                                    .Type = EShaderBufferType::Struct,
                                    .Size = 0,
                                    .Offset = 0,
                                    .Columns = 1,
                                    .Rows = 1,
                                    .Members =
                                        {
                                            {
                                                .Name = "spotLightArray",
                                                .Type = EShaderBufferType::Struct,
                                                .Size = 60,
                                                .Offset = 0,
                                                .Columns = 1,
                                                .Rows = 1,
                                                .Members =
                                                    {

                                                        {
                                                            .Name = "position",
                                                            .Type = EShaderBufferType::Float,
                                                            .Size = 4,
                                                            .Offset = 0,
                                                            .Columns = 1,
                                                            .Rows = 4,

                                                        },
                                                        {
                                                            .Name = "direction",
                                                            .Type = EShaderBufferType::Float,
                                                            .Size = 4,
                                                            .Offset = 16,
                                                            .Columns = 1,
                                                            .Rows = 4,

                                                        },
                                                        {
                                                            .Name = "color",
                                                            .Type = EShaderBufferType::Float,
                                                            .Size = 4,
                                                            .Offset = 32,
                                                            .Columns = 1,
                                                            .Rows = 4,

                                                        },
                                                        {
                                                            .Name = "cutOff",
                                                            .Type = EShaderBufferType::Float,
                                                            .Size = 4,
                                                            .Offset = 48,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "outerCutOff",
                                                            .Type = EShaderBufferType::Float,
                                                            .Size = 4,
                                                            .Offset = 52,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "intensity",
                                                            .Type = EShaderBufferType::Float,
                                                            .Size = 4,
                                                            .Offset = 56,
                                                            .Columns = 1,
                                                            .Rows = 1,
                                                        },
                                                    },
                                            },
                                        },
                                },
                            },
                    },
                    ShaderResource::StorageBuffer{
                        .Set = 1,
                        .Binding = 2,
                        .Parameter =
                            {
                                ShaderParameter{
                                    .Name = "LightBuffer",
                                    .Type = EShaderBufferType::Struct,
                                    .Size = 0,
                                    .Offset = 0,
                                    .Columns = 1,
                                    .Rows = 1,
                                    .Members =
                                        {
                                            {
                                                .Name = "pointLightArray",
                                                .Type = EShaderBufferType::Struct,
                                                .Size = 40,
                                                .Offset = 0,
                                                .Columns = 1,
                                                .Rows = 1,
                                                .Members =
                                                    {

                                                        {
                                                            .Name = "position",
                                                            .Type = EShaderBufferType::Float,
                                                            .Size = 4,
                                                            .Offset = 0,
                                                            .Columns = 1,
                                                            .Rows = 4,

                                                        },
                                                        {
                                                            .Name = "color",
                                                            .Type = EShaderBufferType::Float,
                                                            .Size = 4,
                                                            .Offset = 16,
                                                            .Columns = 1,
                                                            .Rows = 4,

                                                        },
                                                        {
                                                            .Name = "intensity",
                                                            .Type = EShaderBufferType::Float,
                                                            .Size = 4,
                                                            .Offset = 32,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "falloff",
                                                            .Type = EShaderBufferType::Float,
                                                            .Size = 4,
                                                            .Offset = 36,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                    },
                                            },
                                        },
                                },
                            },
                    },
                },
        };
    const VulkanShader::ReflectionData& GotReflection = ShaderResult->GetReflectionData();

    CheckReflection(ExpectedReflection, GotReflection);

    ::Log::Shutdown();
}

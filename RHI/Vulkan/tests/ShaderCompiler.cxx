#include "Engine/Raphael.hxx"

#include "Engine/Core/Log.hxx"

#include <magic_enum.hpp>

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

#define INFO_N_CHECK(Name, Member, Got, Expected)                            \
    INFO(Name " " #Member " : " << Got.Member << " == " << Expected.Member); \
    CHECK(Got.Member == Expected.Member);

static void CheckParameter(const RTTI::FParameter& Expected, const RTTI::FParameter& Got)
{
    INFO_N_CHECK("Parameter", Name, Got, Expected)

    INFO("Type: " << magic_enum::enum_name(Got.Type) << " == " << magic_enum::enum_name(Expected.Type));
    CHECK(Got.Type == Expected.Type);

    INFO_N_CHECK("Parameter", Size, Got, Expected)
    INFO_N_CHECK("Parameter", Offset, Got, Expected)
    INFO_N_CHECK("Parameter", Columns, Got, Expected)
    INFO_N_CHECK("Parameter", Rows, Got, Expected)

    REQUIRE(Got.Members.Size() == Expected.Members.Size());
    for (unsigned i = 0; i < Got.Members.Size(); i++) {
        CheckParameter(Expected.Members[i], Got.Members[i]);
    }
}

static void CheckReflection(const VulkanRHI::RVulkanShader::FReflectionData& ExpectedReflection,
                            const VulkanRHI::RVulkanShader::FReflectionData& GotReflection)
{
    REQUIRE(GotReflection.PushConstants.has_value() == ExpectedReflection.PushConstants.has_value());
    if (GotReflection.PushConstants.has_value() && ExpectedReflection.PushConstants.has_value()) {
        INFO_N_CHECK("Push Constants:", Offset, GotReflection.PushConstants.value(),
                     ExpectedReflection.PushConstants.value());
        INFO_N_CHECK("Push Constants:", Size, GotReflection.PushConstants.value(),
                     ExpectedReflection.PushConstants.value());
        CheckParameter(ExpectedReflection.PushConstants.value().Parameter,
                       GotReflection.PushConstants.value().Parameter);
    }

    REQUIRE(GotReflection.StageInput.Size() == ExpectedReflection.StageInput.Size());
    for (unsigned i = 0; i < GotReflection.StageInput.Size(); i++) {
        INFO_N_CHECK("Stage Input", Name, GotReflection.StageInput[i], GotReflection.StageInput[i])

        INFO("Type: " << magic_enum::enum_name(GotReflection.StageInput[i].Type)
                      << " == " << magic_enum::enum_name(ExpectedReflection.StageInput[i].Type));
        CHECK(GotReflection.StageInput[i].Type == ExpectedReflection.StageInput[i].Type);

        INFO_N_CHECK("Stage Input", Binding, GotReflection.StageInput[i], GotReflection.StageInput[i])
        INFO_N_CHECK("Stage Input", Location, GotReflection.StageInput[i], GotReflection.StageInput[i])
        INFO_N_CHECK("Stage Input", Offset, GotReflection.StageInput[i], GotReflection.StageInput[i])
    }

    REQUIRE(GotReflection.StageOutput.Size() == ExpectedReflection.StageOutput.Size());
    for (unsigned i = 0; i < GotReflection.StageOutput.Size(); i++) {
        INFO_N_CHECK("Stage Output", Name, GotReflection.StageOutput[i], GotReflection.StageOutput[i])

        INFO("Type: " << magic_enum::enum_name(GotReflection.StageOutput[i].Type)
                      << " == " << magic_enum::enum_name(ExpectedReflection.StageOutput[i].Type));
        CHECK(GotReflection.StageOutput[i].Type == ExpectedReflection.StageOutput[i].Type);

        INFO_N_CHECK("Stage Output", Binding, GotReflection.StageOutput[i], GotReflection.StageOutput[i])
        INFO_N_CHECK("Stage Output", Location, GotReflection.StageOutput[i], GotReflection.StageOutput[i])
        INFO_N_CHECK("Stage Output", Offset, GotReflection.StageOutput[i], GotReflection.StageOutput[i])
    }

    REQUIRE(GotReflection.StorageBuffers.Size() == ExpectedReflection.StorageBuffers.Size());
    for (unsigned i = 0; i < GotReflection.StorageBuffers.Size(); i++) {
        INFO_N_CHECK("Storage Buffer", Set, GotReflection.StorageBuffers[i], GotReflection.StorageBuffers[i])
        INFO_N_CHECK("Storage Buffer", Binding, GotReflection.StorageBuffers[i], GotReflection.StorageBuffers[i])
        CheckParameter(ExpectedReflection.StorageBuffers[i].Parameter, GotReflection.StorageBuffers[i].Parameter);
    }

    REQUIRE(GotReflection.UniformBuffers.Size() == ExpectedReflection.UniformBuffers.Size());
    for (unsigned i = 0; i < GotReflection.UniformBuffers.Size(); i++) {
        INFO_N_CHECK("Uniform Buffer", Set, GotReflection.UniformBuffers[i], GotReflection.UniformBuffers[i])
        INFO_N_CHECK("Uniform Buffer", Binding, GotReflection.UniformBuffers[i], GotReflection.UniformBuffers[i])
        CheckParameter(ExpectedReflection.UniformBuffers[i].Parameter, GotReflection.UniformBuffers[i].Parameter);
    }

    CHECK(GotReflection == ExpectedReflection);
}

TEST_CASE("Vulkan Shader Compiler: Simple Compilation")
{
    using namespace VulkanRHI;
    ::Log::Init();

    std::filesystem::path SimpleShaderPath = GetCurrentFilePath() / "test_shaders/SimpleShader.vert";
    FVulkanShaderCompiler Compiler;
    Compiler.SetOptimizationLevel(FVulkanShaderCompiler::EOptimizationLevel::None);

    Ref<RVulkanShader> ShaderResult = Compiler.Get(SimpleShaderPath, false, true);
    REQUIRE(ShaderResult);

    SECTION("Test shader Cache")
    {
        Ref<RVulkanShader> CachedResult = Compiler.Get(SimpleShaderPath, false, true);

        CHECK(ShaderResult == CachedResult);
    }

    CHECK(ShaderResult->GetShaderType() == ERHIShaderType::Vertex);

    const RVulkanShader::FReflectionData ExpectedReflection{
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
                    .Offset = 0,
                },
                {
                    .Name = "outUV",
                    .Type = EVertexElementType::Uint2,
                    .Location = 1,
                    .Offset = 16,
                },
            },
        .PushConstants = {},
    };
    const RVulkanShader::FReflectionData& GotReflection = ShaderResult->GetReflectionData();

    CheckReflection(ExpectedReflection, GotReflection);

    ::Log::Shutdown();
}

BEGIN_PARAMETER_STRUCT(PointLightStruct)
PARAMETER(FVector4, position)
PARAMETER(FVector4, color)
PARAMETER(float, intensity)
PARAMETER(float, falloff)
END_PARAMETER_STRUCT();

BEGIN_PARAMETER_STRUCT(DirectionalLight)
PARAMETER(FVector4, orientation)
PARAMETER(FVector4, color)
PARAMETER(float, intensity)
END_PARAMETER_STRUCT();

BEGIN_PARAMETER_STRUCT(SpotLight)
PARAMETER(FVector4, position)
PARAMETER(FVector4, direction)
PARAMETER(FVector4, color)
PARAMETER(float, cutOff)
PARAMETER(float, outerCutOff)
PARAMETER(float, intensity)
END_PARAMETER_STRUCT();

BEGIN_PARAMETER_STRUCT(MaterialStruct)
PARAMETER(float, alphaCutOff)
PARAMETER(float, metallic)
PARAMETER(float, roughness)
PARAMETER(FVector4, baseColor)
PARAMETER(FVector4, baseColorFactor)
PARAMETER(FVector4, emissiveFactor)
PARAMETER(int, baseColorTexture)
PARAMETER(int, metallicRoughnessTexture)
PARAMETER(int, normalTexture)
PARAMETER(int, occlusionTexture)
PARAMETER(int, emissiveTexture)
PARAMETER(int, specularGlossinessTexture)
PARAMETER(int, diffuseTexture)
END_PARAMETER_STRUCT();

BEGIN_PARAMETER_STRUCT(PushConstantStruct)
PARAMETER(int, pointLightCount)
PARAMETER(int, directLightCount)
PARAMETER(int, spotLightCount)
PARAMETER(FVector3, position)
END_PARAMETER_STRUCT();

TEST_CASE("Vulkan Shader Compiler: Complex Compilation")
{
    using namespace VulkanRHI;
    ::Log::Init();

    std::filesystem::path SimpleShaderPath = GetCurrentFilePath() / "test_shaders/TestComplex.frag";
    FVulkanShaderCompiler Compiler;
    Compiler.SetOptimizationLevel(VulkanRHI::FVulkanShaderCompiler::EOptimizationLevel::None);

    Ref<RVulkanShader> ShaderResult = Compiler.Get(SimpleShaderPath, false, true);
    REQUIRE(ShaderResult);

    CHECK(ShaderResult->GetShaderType() == ERHIShaderType::Fragment);

    const uint32 ExpectedPushConstantOffset = 64;
    const ShaderResource::FPushConstantRange ExpectedPushConstant{
        .Offset = ExpectedPushConstantOffset,
        .Size = ExpectedPushConstantOffset + sizeof(PushConstantStruct),
        .Parameter =
            {
                .Name = "push",
                .Type = ::RTTI::EParameterType::Struct,
                .Size = ExpectedPushConstantOffset + sizeof(PushConstantStruct),
                .Offset = ExpectedPushConstantOffset,
                .Columns = 1,
                .Rows = 1,
                .Members =
                    {
                        {
                            .Name = "push",
                            .Type = ::RTTI::EParameterType::Struct,
                            .Size = sizeof(PushConstantStruct),
                            .Offset = ExpectedPushConstantOffset,
                            .Columns = 1,
                            .Rows = 1,
                            .Members =
                                {
                                    {
                                        .Name = "pointLightCount",
                                        .Type = ::RTTI::EParameterType::Uint32,
                                        .Size = 4,
                                        .Offset = 0,
                                        .Columns = 1,
                                        .Rows = 1,
                                    },
                                    {
                                        .Name = "directLightCount",
                                        .Type = ::RTTI::EParameterType::Uint32,
                                        .Size = 4,
                                        .Offset = 4,
                                        .Columns = 1,
                                        .Rows = 1,
                                    },
                                    {
                                        .Name = "spotLightCount",
                                        .Type = ::RTTI::EParameterType::Uint32,
                                        .Size = 4,
                                        .Offset = 8,
                                        .Columns = 1,
                                        .Rows = 1,
                                    },
                                    {
                                        .Name = "position",
                                        .Type = ::RTTI::EParameterType::Float,
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

    const RVulkanShader::FReflectionData
        ExpectedReflection{
            .StageInput =
                {
                    {
                        .Name = "fragPosition",
                        .Type = EVertexElementType::Float3,
                        .Location = 0,
                        .Offset = 8,
                    },
                    {
                        .Name = "fragNormal",
                        .Type = EVertexElementType::Float3,
                        .Location = 1,
                        .Offset = 20,
                    },
                    {
                        .Name = "fragTextCoords",
                        .Type = EVertexElementType::Float2,
                        .Location = 2,
                        .Offset = 0,
                    },
                    {
                        .Name = "fragColor",
                        .Type = EVertexElementType::Float3,
                        .Location = 3,
                        .Offset = 36,
                    },
                    {
                        .Name = "fragTangent",
                        .Type = EVertexElementType::Float4,
                        .Location = 4,
                        .Offset = 48,
                    },
                    {
                        .Name = "materialIndex",
                        .Type = EVertexElementType::Uint1,
                        .Location = 5,
                        .Offset = 32,
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
                    ShaderResource::FStorageBuffer{
                        .Set = 2,
                        .Binding = 1,
                        .Parameter =
                            {
                                ::RTTI::FParameter{
                                    .Name = "ObjectMaterials",
                                    .Type = ::RTTI::EParameterType::Struct,
                                    .Size = 0,
                                    .Offset = 0,
                                    .Columns = 1,
                                    .Rows = 1,
                                    .Members =
                                        {
                                            {
                                                .Name = "materials",
                                                .Type = ::RTTI::EParameterType::Struct,
                                                .Size = 92,
                                                .Offset = 0,
                                                .Columns = 1,
                                                .Rows = 1,
                                                .Members =
                                                    {
                                                        {
                                                            .Name = "alphaCutOff",
                                                            .Type = ::RTTI::EParameterType::Float,
                                                            .Size = 4,
                                                            .Offset = 0,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "metallic",
                                                            .Type = ::RTTI::EParameterType::Float,
                                                            .Size = 4,
                                                            .Offset = 4,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "roughness",
                                                            .Type = ::RTTI::EParameterType::Float,
                                                            .Size = 4,
                                                            .Offset = 8,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "baseColor",
                                                            .Type = ::RTTI::EParameterType::Float,
                                                            .Size = 4,
                                                            .Offset = 16,
                                                            .Columns = 1,
                                                            .Rows = 4,

                                                        },
                                                        {
                                                            .Name = "baseColorFactor",
                                                            .Type = ::RTTI::EParameterType::Float,
                                                            .Size = 4,
                                                            .Offset = 32,
                                                            .Columns = 1,
                                                            .Rows = 4,

                                                        },
                                                        {
                                                            .Name = "emissiveFactor",
                                                            .Type = ::RTTI::EParameterType::Float,
                                                            .Size = 4,
                                                            .Offset = 48,
                                                            .Columns = 1,
                                                            .Rows = 4,

                                                        },
                                                        {
                                                            .Name = "baseColorTexture",
                                                            .Type = ::RTTI::EParameterType::Int32,
                                                            .Size = 4,
                                                            .Offset = 64,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "metallicRoughnessTexture",
                                                            .Type = ::RTTI::EParameterType::Int32,
                                                            .Size = 4,
                                                            .Offset = 68,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "normalTexture",
                                                            .Type = ::RTTI::EParameterType::Int32,
                                                            .Size = 4,
                                                            .Offset = 72,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "occlusionTexture",
                                                            .Type = ::RTTI::EParameterType::Int32,
                                                            .Size = 4,
                                                            .Offset = 76,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "emissiveTexture",
                                                            .Type = ::RTTI::EParameterType::Int32,
                                                            .Size = 4,
                                                            .Offset = 80,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "specularGlossinessTexture",
                                                            .Type = ::RTTI::EParameterType::Int32,
                                                            .Size = 4,
                                                            .Offset = 84,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "diffuseTexture",
                                                            .Type = ::RTTI::EParameterType::Int32,
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
                    ShaderResource::FStorageBuffer{
                        .Set = 1,
                        .Binding = 0,
                        .Parameter =
                            ::RTTI::FParameter{
                                .Name = "DirectLight",
                                .Type = ::RTTI::EParameterType::Struct,
                                .Size = 0,
                                .Offset = 0,
                                .Columns = 1,
                                .Rows = 1,
                                .Members =
                                    {
                                        {
                                            .Name = "directionalLightArray",
                                            .Type = ::RTTI::EParameterType::Struct,
                                            .Size = 36,
                                            .Offset = 0,
                                            .Columns = 1,
                                            .Rows = 1,
                                            .Members =
                                                {
                                                    {
                                                        .Name = "orientation",
                                                        .Type = ::RTTI::EParameterType::Float,
                                                        .Size = 4,
                                                        .Offset = 0,
                                                        .Columns = 1,
                                                        .Rows = 4,

                                                    },
                                                    {
                                                        .Name = "color",
                                                        .Type = ::RTTI::EParameterType::Float,
                                                        .Size = 4,
                                                        .Offset = 16,
                                                        .Columns = 1,
                                                        .Rows = 4,

                                                    },
                                                    {
                                                        .Name = "intensity",
                                                        .Type = ::RTTI::EParameterType::Float,
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

                    ShaderResource::FStorageBuffer{
                        .Set = 1,
                        .Binding = 1,
                        .Parameter =
                            {
                                ::RTTI::FParameter{
                                    .Name = "SpoLight",
                                    .Type = ::RTTI::EParameterType::Struct,
                                    .Size = 0,
                                    .Offset = 0,
                                    .Columns = 1,
                                    .Rows = 1,
                                    .Members =
                                        {
                                            {
                                                .Name = "spotLightArray",
                                                .Type = ::RTTI::EParameterType::Struct,
                                                .Size = 60,
                                                .Offset = 0,
                                                .Columns = 1,
                                                .Rows = 1,
                                                .Members =
                                                    {

                                                        {
                                                            .Name = "position",
                                                            .Type = ::RTTI::EParameterType::Float,
                                                            .Size = 4,
                                                            .Offset = 0,
                                                            .Columns = 1,
                                                            .Rows = 4,

                                                        },
                                                        {
                                                            .Name = "direction",
                                                            .Type = ::RTTI::EParameterType::Float,
                                                            .Size = 4,
                                                            .Offset = 16,
                                                            .Columns = 1,
                                                            .Rows = 4,

                                                        },
                                                        {
                                                            .Name = "color",
                                                            .Type = ::RTTI::EParameterType::Float,
                                                            .Size = 4,
                                                            .Offset = 32,
                                                            .Columns = 1,
                                                            .Rows = 4,

                                                        },
                                                        {
                                                            .Name = "cutOff",
                                                            .Type = ::RTTI::EParameterType::Float,
                                                            .Size = 4,
                                                            .Offset = 48,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "outerCutOff",
                                                            .Type = ::RTTI::EParameterType::Float,
                                                            .Size = 4,
                                                            .Offset = 52,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "intensity",
                                                            .Type = ::RTTI::EParameterType::Float,
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
                    ShaderResource::FStorageBuffer{
                        .Set = 1,
                        .Binding = 2,
                        .Parameter =
                            {
                                ::RTTI::FParameter{
                                    .Name = "LightBuffer",
                                    .Type = ::RTTI::EParameterType::Struct,
                                    .Size = 0,
                                    .Offset = 0,
                                    .Columns = 1,
                                    .Rows = 1,
                                    .Members =
                                        {
                                            {
                                                .Name = "pointLightArray",
                                                .Type = ::RTTI::EParameterType::Struct,
                                                .Size = 40,
                                                .Offset = 0,
                                                .Columns = 1,
                                                .Rows = 1,
                                                .Members =
                                                    {

                                                        {
                                                            .Name = "position",
                                                            .Type = ::RTTI::EParameterType::Float,
                                                            .Size = 4,
                                                            .Offset = 0,
                                                            .Columns = 1,
                                                            .Rows = 4,

                                                        },
                                                        {
                                                            .Name = "color",
                                                            .Type = ::RTTI::EParameterType::Float,
                                                            .Size = 4,
                                                            .Offset = 16,
                                                            .Columns = 1,
                                                            .Rows = 4,

                                                        },
                                                        {
                                                            .Name = "intensity",
                                                            .Type = ::RTTI::EParameterType::Float,
                                                            .Size = 4,
                                                            .Offset = 32,
                                                            .Columns = 1,
                                                            .Rows = 1,

                                                        },
                                                        {
                                                            .Name = "falloff",
                                                            .Type = ::RTTI::EParameterType::Float,
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
    const RVulkanShader::FReflectionData& GotReflection = ShaderResult->GetReflectionData();

    CheckReflection(ExpectedReflection, GotReflection);

    SECTION("Test Serialization")
    {
        {
            Serialization::FFileStreamWriter Writer("test.txt");
            Writer.WriteObject(ExpectedReflection);
            Writer.Flush();
        }
        RVulkanShader::FReflectionData GotExpectedReflection;
        {
            Serialization::FFileStreamReader Reader("test.txt");
            CHECK(Reader);
            CHECK(Reader.IsGood());

            Reader.ReadObject(GotExpectedReflection);
        }
        CHECK(GotExpectedReflection == ExpectedReflection);
    }

    ::Log::Shutdown();
}

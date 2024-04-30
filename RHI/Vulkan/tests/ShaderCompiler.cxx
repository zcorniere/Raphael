#include "Engine/Raphael.hxx"

#include "Engine/Core/Log.hxx"

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

TEST_CASE("Vulkan Shader Compiler: Complex Compilation")
{
    using namespace VulkanRHI;
    ::Log::Init();

    std::filesystem::path SimpleShaderPath = GetCurrentFilePath() / "test_shaders/TestComplex.frag";
    VulkanShaderCompiler Compiler;

    Ref<VulkanShader> ShaderResult = Compiler.Get(SimpleShaderPath);
    REQUIRE(ShaderResult);

    CHECK(ShaderResult->GetShaderType() == ERHIShaderType::Pixel);

    struct ShaderPushConstantStruct {
        uint32 pointLightCount;
        uint32 directLightCount;
        uint32 spotLightCount;
        float position[3];
        float __Padding;
    };
    const uint32 ExpectedPushConstantOffset = 64;
    const ShaderResource::PushConstantRange ExpectedPushConstant{
        .Offset = ExpectedPushConstantOffset,
        .Size = ExpectedPushConstantOffset + sizeof(ShaderPushConstantStruct),
    };

    const VulkanShader::ReflectionData ExpectedReflection{
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
    };
    const VulkanShader::ReflectionData& GotReflection = ShaderResult->GetReflectionData();

    CheckReflection(ExpectedReflection, GotReflection);

    ::Log::Shutdown();
}

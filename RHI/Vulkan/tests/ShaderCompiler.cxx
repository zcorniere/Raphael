#include "Engine/Core/Log.hxx"
#include "Engine/Raphael.hxx"

#include "VulkanRHI/VulkanShaderCompiler.hxx"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

std::filesystem::path GetCurrentFilePath()
{
    std::source_location Location = std::source_location::current();
    std::filesystem::path File(Location.file_name());

    return File.parent_path();
}

TEST_CASE("Vulkan Shader Compiler: Simple Compilation")
{
    using namespace VulkanRHI;
    ::Log::Init();

    std::filesystem::path SimpleShaderPath = GetCurrentFilePath() / "test_shaders/SimpleShader.vert";
    VulkanShaderCompiler Compiler;

    Ref<VulkanShader> ShaderResult = Compiler.Get(SimpleShaderPath);
    REQUIRE(ShaderResult);

    SECTION("Test shader Cache")
    {
        Ref<VulkanShader> CachedResult = Compiler.Get(SimpleShaderPath);

        CHECK(ShaderResult == CachedResult);
    }

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

    ::Log::Shutdown();
}

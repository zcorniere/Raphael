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

void CheckShaderIO(const VulkanRHI::ShaderResource::StageIO& Got, const VulkanRHI::ShaderResource::StageIO& Expected)
{
    CHECK(Got.Name == Expected.Name);
    CHECK(Got.Type == Expected.Type);
    CHECK(Got.Location == Expected.Location);
}

TEST_CASE("Test simple shader Compilation", "[RHI][Vulkan]")
{
    ::Log::Init();

    std::filesystem::path SimpleShaderPath = GetCurrentFilePath() / "test_shaders/SimpleShader.vert";
    VulkanRHI::VulkanShaderCompiler Compiler;

    Ref<VulkanRHI::VulkanShader> ShaderResult = Compiler.Get(SimpleShaderPath);
    REQUIRE(ShaderResult);

    SECTION("Test shader Cache")
    {
        Ref<VulkanRHI::VulkanShader> CachedResult = Compiler.Get(SimpleShaderPath);

        CHECK(ShaderResult == CachedResult);
    }

    // Test the StageInput
    REQUIRE(ShaderResult->GetReflectionData().StageInput.Size() == 1);
    CheckShaderIO(ShaderResult->GetReflectionData().StageInput[0], {
                                                                       .Name = "inVertex",
                                                                       .Type = EVertexElementType::Float4,
                                                                       .Location = 0,
                                                                   });

    // Test the StageOutput
    REQUIRE(ShaderResult->GetReflectionData().StageOutput.Size() == 2);
    CheckShaderIO(ShaderResult->GetReflectionData().StageOutput[0], {
                                                                        .Name = "outVertexPos",
                                                                        .Type = EVertexElementType::Float4,
                                                                        .Location = 0,
                                                                    });
    CheckShaderIO(ShaderResult->GetReflectionData().StageOutput[1], {
                                                                        .Name = "outUV",
                                                                        .Type = EVertexElementType::Uint2,
                                                                        .Location = 1,
                                                                    });

    ::Log::Shutdown();
}

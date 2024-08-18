#include "Engine/Raphael.hxx"

#include "Engine/Core/RHI/RHIShaderParameters.hxx"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

BEGIN_SHADER_PARAMETER_STRUCT(FShaderStruct)
SHADER_PARAMETER(int32, TestValue)
SHADER_PARAMETER(float, TestFloat)
SHADER_PARAMETER(FVector3, TestVec3)
SHADER_PARAMETER(FMatrix4, TestMat4)
END_SHADER_PARAMETER_STRUCT();

TEST_CASE("ShaderParameterStruct")
{
    FShaderStruct TestStruct;
    TestStruct.TestValue = 42;
    TestStruct.TestFloat = 69.0f;

    REQUIRE(sizeof(FShaderStruct) % 4 == 0);    // Make sure the struct produced is aligned

    Array<ShaderParameter> Members = FShaderStruct::GetMembers();
    CHECK(Members.Size() == 4);

    REQUIRE(Members[0].Name == "TestValue");
    REQUIRE(Members[0].Offset == 0);
    REQUIRE(Members[0].Size == sizeof(int));
    REQUIRE(Members[0].Type == EShaderBufferType::Int32);
    REQUIRE(Members[0].Columns == 1);
    REQUIRE(Members[0].Rows == 1);

    REQUIRE(Members[1].Name == "TestFloat");
    REQUIRE(Members[1].Offset == sizeof(int));
    REQUIRE(Members[1].Size == sizeof(float));
    REQUIRE(Members[1].Type == EShaderBufferType::Float);
    REQUIRE(Members[1].Columns == 1);
    REQUIRE(Members[1].Rows == 1);

    // The odd aligned vec3 is automatically padded to vec4
    REQUIRE(Members[2].Name == "TestVec3");
    REQUIRE(Members[2].Offset == offsetof(FShaderStruct, TestVec3));
    REQUIRE(Members[2].Size == sizeof(FVector4));    // <-- hence the sizeof vec4
    REQUIRE(Members[2].Type == EShaderBufferType::Float);
    REQUIRE(Members[2].Columns == 1);
    REQUIRE(Members[2].Rows == 3);

    REQUIRE(Members[3].Name == "TestMat4");
    REQUIRE(Members[3].Offset == offsetof(FShaderStruct, TestMat4));
    REQUIRE(Members[3].Size == sizeof(FMatrix4));
    REQUIRE(Members[3].Type == EShaderBufferType::Float);
    REQUIRE(Members[3].Columns == 4);
    REQUIRE(Members[3].Rows == 4);
}

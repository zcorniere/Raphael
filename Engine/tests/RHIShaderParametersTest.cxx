#include "Engine/Raphael.hxx"

#include "Engine/Core/RHI/RHIShaderParameters.hxx"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

BEGIN_PARAMETER_STRUCT(FShaderStruct)
PARAMETER(int32, TestValue)
PARAMETER(float, TestFloat)
PARAMETER(FVector3, TestVec3)
PARAMETER(FMatrix4, TestMat4)
END_PARAMETER_STRUCT();

TEST_CASE("ShaderParameterStruct")
{
    FShaderStruct TestStruct;
    TestStruct.TestValue = 42;
    TestStruct.TestFloat = 69.0f;

    REQUIRE(sizeof(FShaderStruct) % 4 == 0);    // Make sure the struct produced is aligned

    TArray<FParameter> Members = FShaderStruct::GetMembers();
    CHECK(Members.Size() == 4);

    CHECK(Members[0].Name == "TestValue");
    CHECK(Members[0].Offset == 0);
    CHECK(Members[0].Size == sizeof(int));
    CHECK(Members[0].Type == EShaderBufferType::Int32);
    CHECK(Members[0].Columns == 1);
    CHECK(Members[0].Rows == 1);

    CHECK(Members[1].Name == "TestFloat");
    CHECK(Members[1].Offset == sizeof(int));
    CHECK(Members[1].Size == sizeof(float));
    CHECK(Members[1].Type == EShaderBufferType::Float);
    CHECK(Members[1].Columns == 1);
    CHECK(Members[1].Rows == 1);

    // The odd aligned vec3 should be automatically padded to vec4
    CHECK(Members[2].Name == "TestVec3");
    CHECK(Members[2].Offset == offsetof(FShaderStruct, TestVec3));
    CHECK(Members[2].Size == sizeof(FVector4));    // <-- hence the sizeof vec4
    CHECK(Members[2].Type == EShaderBufferType::Float);
    CHECK(Members[2].Columns == 1);
    CHECK(Members[2].Rows == 3);

    CHECK(Members[3].Name == "TestMat4");
    CHECK(Members[3].Offset == offsetof(FShaderStruct, TestMat4));
    CHECK(Members[3].Size == sizeof(FMatrix4));
    CHECK(Members[3].Type == EShaderBufferType::Float);
    CHECK(Members[3].Columns == 4);
    CHECK(Members[3].Rows == 4);

    // Check if the struct is the same size as the sum of its members
    unsigned offsetA = offsetof(FShaderStruct, TestValue);
    unsigned offsetB = offsetof(FShaderStruct, TestFloat);
    unsigned offsetC = offsetof(FShaderStruct, TestVec3);
    unsigned offsetD = offsetof(FShaderStruct, TestMat4);

    // Calculate padding between members
    unsigned paddingAB = offsetB - (offsetA + sizeof(int));
    unsigned paddingBC = offsetC - (offsetB + sizeof(float));
    unsigned paddingCD = offsetD - (offsetC + sizeof(FVector4));
    unsigned paddingEnd = sizeof(FShaderStruct) - (offsetC + sizeof(FMatrix4));

    INFO("Padding between members: " << paddingAB << ", " << paddingBC << ", " << paddingCD << ",  " << paddingEnd);
    CHECK(sizeof(FShaderStruct) ==
          Members[0].Size + paddingAB + Members[1].Size + paddingBC + Members[2].Size + paddingCD + Members[3].Size);
}

#include "Engine/Raphael.hxx"

#include "Engine/Core/RTTI/RTTIParameter.hxx"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

namespace TestTypes
{
BEGIN_PARAMETER_STRUCT(FShaderStruct)
PARAMETER(int32, TestValue)
PARAMETER(float, TestFloat)
PARAMETER(FVector3, TestVec3)
PARAMETER(FMatrix4, TestMat4)
END_PARAMETER_STRUCT();

BEGIN_UNALIGNED_PARAMETER_STRUCT(FVertex)
PARAMETER(FVector3, Position)
PARAMETER(FVector3, Normal)
END_PARAMETER_STRUCT();

}    // namespace TestTypes

TEST_CASE("Test Parametter struct")
{
    using namespace RTTI;
    using namespace TestTypes;

    FShaderStruct TestStruct;
    TestStruct.TestValue = 42;
    TestStruct.TestFloat = 69.0f;

    REQUIRE(sizeof(FShaderStruct) % 4 == 0);    // Make sure the struct produced is aligned

    TArray<FParameter> Members = FShaderStruct::GetMembers();
    CHECK(Members.Size() == 4);

    CHECK(Members[0].Name == "TestValue");
    CHECK(Members[0].Offset == 0);
    CHECK(Members[0].Size == sizeof(int));
    CHECK(Members[0].Type == EParameterType::Int32);
    CHECK(Members[0].Columns == 1);
    CHECK(Members[0].Rows == 1);

    CHECK(Members[1].Name == "TestFloat");
    CHECK(Members[1].Offset == sizeof(int));
    CHECK(Members[1].Size == sizeof(float));
    CHECK(Members[1].Type == EParameterType::Float);
    CHECK(Members[1].Columns == 1);
    CHECK(Members[1].Rows == 1);

    // The odd aligned vec3 should be automatically padded to vec4
    CHECK(Members[2].Name == "TestVec3");
    CHECK(Members[2].Offset == offsetof(FShaderStruct, TestVec3));
    CHECK(Members[2].Size == sizeof(FVector4));    // <-- hence the sizeof vec4
    CHECK(Members[2].Type == EParameterType::Float);
    CHECK(Members[2].Columns == 1);
    CHECK(Members[2].Rows == 3);

    CHECK(Members[3].Name == "TestMat4");
    CHECK(Members[3].Offset == offsetof(FShaderStruct, TestMat4));
    CHECK(Members[3].Size == sizeof(FMatrix4));
    CHECK(Members[3].Type == EParameterType::Float);
    CHECK(Members[3].Columns == 4);
    CHECK(Members[3].Rows == 4);

    // Check if the struct is the same size as the sum of its members
    int offsetA = offsetof(FShaderStruct, TestValue);
    int offsetB = offsetof(FShaderStruct, TestFloat);
    int offsetC = offsetof(FShaderStruct, TestVec3);
    int offsetD = offsetof(FShaderStruct, TestMat4);

    // Calculate padding between members
    int paddingAB = offsetB - (offsetA + sizeof(int));
    int paddingBC = offsetC - (offsetB + sizeof(float));
    int paddingCD = offsetD - (offsetC + sizeof(FVector4));
    int paddingEnd = sizeof(FShaderStruct) - (offsetD + sizeof(FMatrix4));

    INFO("Padding between members: " << paddingAB << ", " << paddingBC << ", " << paddingCD << ",  " << paddingEnd);
    CHECK(sizeof(FShaderStruct) == Members[0].Size + paddingAB + Members[1].Size + paddingBC + Members[2].Size +
                                       paddingCD + Members[3].Size + paddingEnd);
}

TEST_CASE("Test Parametter info with no align option")
{
    using namespace RTTI;
    using namespace TestTypes;

    struct FExpectedVertexMemLayout
    {
        FVector3 Position;
        FVector3 Normal;
    };

    TArray<FParameter> Members = FVertex::GetMembers();

    CHECK(Members.Size() == 2);

    CHECK(Members[0].Name == "Position");
    CHECK(Members[0].Offset == 0);
    CHECK(Members[0].Size == sizeof(FVector3));
    CHECK(Members[0].Type == EParameterType::Float);
    CHECK(Members[0].Columns == 1);
    CHECK(Members[0].Rows == 3);

    CHECK(Members[1].Name == "Normal");
    CHECK(Members[1].Offset == sizeof(FVector3));
    CHECK(Members[1].Size == sizeof(FVector3));
    CHECK(Members[1].Type == EParameterType::Float);
    CHECK(Members[1].Columns == 1);
    CHECK(Members[1].Rows == 3);

    CHECK(alignof(FVertex) == alignof(FExpectedVertexMemLayout));
    CHECK(sizeof(FVertex) == sizeof(FExpectedVertexMemLayout));
    check(offsetof(FVertex, Position) == offsetof(FExpectedVertexMemLayout, Position));
    check(offsetof(FVertex, Normal) == offsetof(FExpectedVertexMemLayout, Normal));
}

#include "Engine/Raphael.hxx"

#include "Engine/Core/RHI/RHIShaderParmeters.hxx"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

BEGIN_SHADER_PARAMETER_STRUCT(FShaderStruct)
SHADER_PARAMETER(int, TestValue)
SHADER_PARAMETER(float, TestFloat)
SHADER_PARAMETER(glm::vec3, TestVec3)
END_SHADER_PARAMETER_STRUCT();

TEST_CASE("ShaderParameterStruct")
{
    FShaderStruct TestStruct;
    TestStruct.TestValue = 42;
    TestStruct.TestFloat = 69.0f;

    REQUIRE(sizeof(FShaderStruct) % 4 == 0);    // Make sure the struct produced is aligned

    Array<ShaderParameter> Members = FShaderStruct::GetMembers();
    REQUIRE(Members.Size() == 3);

    REQUIRE(Members[0].Name == "TestValue");
    REQUIRE(Members[0].Offset == 0);
    REQUIRE(Members[0].Size == sizeof(int));

    REQUIRE(Members[1].Name == "TestFloat");
    REQUIRE(Members[1].Offset == sizeof(int));
    REQUIRE(Members[1].Size == sizeof(float));

    // The odd aligned vec3 is automatically padded to vec4
    REQUIRE(Members[2].Name == "TestVec3");
    REQUIRE(Members[2].Offset == offsetof(FShaderStruct, TestVec3));
    REQUIRE(Members[2].Size == sizeof(glm::vec4));    // <-- hence the sizeof vec4
}

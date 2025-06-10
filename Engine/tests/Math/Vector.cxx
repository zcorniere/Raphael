#include "Engine/Raphael.hxx"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

TEMPLATE_TEST_CASE("Vector", "[Math][Vector]", float, double, unsigned, int)
{
    TVector4<TestType> Vec4{1, 2, 3, 4};
    REQUIRE(Vec4.Length == 4);
    CHECK(Vec4[0] == 1);
    CHECK(Vec4.x == 1);
    CHECK(Vec4[1] == 2);
    CHECK(Vec4.y == 2);
    CHECK(Vec4[2] == 3);
    CHECK(Vec4.z == 3);
    CHECK(Vec4[3] == 4);
    CHECK(Vec4.w == 4);

    TVector4<TestType> Vec4Copy = Vec4;
    CHECK(Vec4 == Vec4Copy);
    CHECK(Vec4 != TVector4<TestType>{1, 2, 3, 5});

    SECTION("Vector multiplication")
    {
        TVector4<TestType> Vec4Mul = Vec4 * TVector4<TestType>{2, 2, 2, 2};
        CHECK(Vec4Mul == TVector4<TestType>{2, 4, 6, 8});
    }
    SECTION("Vector subtraction")
    {
        TVector4<TestType> Vec4Sub = Vec4 - TVector4<TestType>{1, 1, 1, 1};
        CHECK(Vec4Sub == TVector4<TestType>{0, 1, 2, 3});
    }

    SECTION("Vector addition")
    {
        TVector4<TestType> Vec4Add = Vec4 + TVector4<TestType>{1, 1, 1, 1};
        CHECK(Vec4Add == TVector4<TestType>{2, 3, 4, 5});
    }
}

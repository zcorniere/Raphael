#include "Engine/Raphael.hxx"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Vector")
{
    UVector4 Vec4{1, 2, 3, 4};
    REQUIRE(Vec4.Length == 4);
    CHECK(Vec4[0] == 1);
    CHECK(Vec4[1] == 2);
    CHECK(Vec4[2] == 3);
    CHECK(Vec4[3] == 4);

    UVector4 Vec4Copy = Vec4;
    CHECK(Vec4 == Vec4Copy);
}

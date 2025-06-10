#include "Engine/Raphael.hxx"

#include "Engine/Math/Math.hxx"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "Common.hxx"

TEMPLATE_TEST_CASE("Degrees and Radians convertion", "[Math][DegreesRadians]", float, double)
{
    const TestType Epsilon = TEpsilon<TestType>::Value;

    SECTION("Degree to Radian")
    {
        const TestType Value = GENERATE(take(10, random(-180.0f, 180.0f)));
        const TestType Result = Math::DegreeToRadian(Value);
        const TestType ExpectedResult = glm::radians(Value);

        CHECK_THAT(Result, Catch::Matchers::WithinRel(ExpectedResult, Epsilon));
    }

    SECTION("Radian to Degree")
    {
        const TestType Value = GENERATE(take(10, random(-1.0f, 1.0f)));
        const TestType Result = Math::RadianToDegree(Value);
        const TestType ExpectedResult = glm::degrees(Value);

        CHECK_THAT(Result, Catch::Matchers::WithinRel(ExpectedResult, Epsilon));
    }
}

TEMPLATE_TEST_CASE("Dot product", "[Math][DotProduct]", float, double)
{
    const TestType Epsilon = TEpsilon<TestType>::Value;

    SECTION("Dot product 2D")
    {
        const glm::vec<2, TestType> _v1{GENERATE(take(1, random(-50.0f, 50.0f))),
                                        GENERATE(take(1, random(-50.0f, 50.0f)))};
        const glm::vec<2, TestType> _v2{GENERATE(take(1, random(-50.0f, 50.0f))),
                                        GENERATE(take(1, random(-50.0f, 50.0f)))};

        const TVector2<TestType> v1(_v1.x, _v1.y);
        const TVector2<TestType> v2(_v2.x, _v2.y);

        const TestType Result = Math::Dot(v1, v2);
        const TestType ExpectedResult = glm::dot(_v1, _v2);

        CHECK_THAT(Result, Catch::Matchers::WithinRel(ExpectedResult, Epsilon));
    }

    SECTION("Dot product 3D", "[Math][DotProduct][3D]")
    {
        const glm::vec<3, TestType> _v1{GENERATE(take(1, random(-50.0f, 50.0f))),
                                        GENERATE(take(1, random(-50.0f, 50.0f))),
                                        GENERATE(take(1, random(-50.0f, 50.0f)))};
        const glm::vec<3, TestType> _v2{GENERATE(take(1, random(-50.0f, 50.0f))),
                                        GENERATE(take(1, random(-50.0f, 50.0f))),
                                        GENERATE(take(1, random(-50.0f, 50.0f)))};

        const TVector3<TestType> v1(_v1.x, _v1.y, _v1.z);
        const TVector3<TestType> v2(_v2.x, _v2.y, _v2.z);

        const TestType Result = Math::Dot(v1, v2);
        const TestType ExpectedResult = glm::dot(_v1, _v2);

        CHECK_THAT(Result, Catch::Matchers::WithinRel(ExpectedResult, Epsilon));
    }
}

TEMPLATE_TEST_CASE("Normalize", "[Math][Normalize]", float, double)
{
    const TestType Epsilon = TEpsilon<TestType>::Value;

    SECTION("Normalize 2D")
    {
        const glm::vec<2, TestType> _v1{GENERATE(take(1, random(-50.0f, 50.0f))),
                                        GENERATE(take(1, random(-50.0f, 50.0f)))};

        const TVector2<TestType> v1(_v1.x, _v1.y);

        const TVector2<TestType> Result = Math::Normalize(v1);
        const glm::vec<2, TestType> ExpectedResult = glm::normalize(_v1);

        CHECK_THAT(Result.x, Catch::Matchers::WithinRel(ExpectedResult.x, Epsilon));
        CHECK_THAT(Result.y, Catch::Matchers::WithinRel(ExpectedResult.y, Epsilon));
    }

    SECTION("Normalize 3D")
    {
        const glm::vec<3, TestType> _v1{GENERATE(take(1, random(-50.0f, 50.0f))),
                                        GENERATE(take(1, random(-50.0f, 50.0f))),
                                        GENERATE(take(1, random(-50.0f, 50.0f)))};

        const TVector3<TestType> v1(_v1.x, _v1.y, _v1.z);

        const TVector3<TestType> Result = Math::Normalize(v1);
        const glm::vec<3, TestType> ExpectedResult = glm::normalize(_v1);

        CHECK_THAT(Result.x, Catch::Matchers::WithinRel(ExpectedResult.x, Epsilon));
        CHECK_THAT(Result.y, Catch::Matchers::WithinRel(ExpectedResult.y, Epsilon));
        CHECK_THAT(Result.z, Catch::Matchers::WithinRel(ExpectedResult.z, Epsilon));
    }

    SECTION("Normalize 4D")
    {
        const glm::vec<4, TestType> _v1{
            GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
            GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f)))};

        const TVector4<TestType> v1(_v1.x, _v1.y, _v1.z, _v1.w);

        const TVector4<TestType> Result = Math::Normalize(v1);
        const glm::vec<4, TestType> ExpectedResult = glm::normalize(_v1);

        CHECK_THAT(Result.x, Catch::Matchers::WithinRel(ExpectedResult.x, Epsilon));
        CHECK_THAT(Result.y, Catch::Matchers::WithinRel(ExpectedResult.y, Epsilon));
        CHECK_THAT(Result.z, Catch::Matchers::WithinRel(ExpectedResult.z, Epsilon));
        CHECK_THAT(Result.w, Catch::Matchers::WithinRel(ExpectedResult.w, Epsilon));
    }
}

TEMPLATE_TEST_CASE("Cross product", "[Math][CrossProduct]", float, double)
{
    const TestType Epsilon = TEpsilon<TestType>::Value;

    const glm::vec<3, TestType> _v1{GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                                    GENERATE(take(1, random(-50.0f, 50.0f)))};
    const glm::vec<3, TestType> _v2{GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                                    GENERATE(take(1, random(-50.0f, 50.0f)))};

    const TVector3<TestType> v1(_v1.x, _v1.y, _v1.z);
    const TVector3<TestType> v2(_v2.x, _v2.y, _v2.z);

    const TVector3<TestType> Result = Math::Cross(v1, v2);
    const glm::vec<3, TestType> ExpectedResult = glm::cross(_v1, _v2);

    CHECK_THAT(Result.x, Catch::Matchers::WithinRel(ExpectedResult.x, Epsilon));
    CHECK_THAT(Result.y, Catch::Matchers::WithinRel(ExpectedResult.y, Epsilon));
    CHECK_THAT(Result.z, Catch::Matchers::WithinRel(ExpectedResult.z, Epsilon));
}

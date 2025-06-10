#include "Engine/Raphael.hxx"

#include "Engine/Math/Quaternion.hxx"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "Common.hxx"

TEMPLATE_TEST_CASE("Quaternion operations", "[Math][Quaternion]", float, double)
{
    Catch::StringMaker<TestType>::precision = 25;

    const TestType Epsilon = TEpsilon<TestType>::Value;

    const glm::qua<TestType, glm::defaultp> _q1{
        GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
        GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f)))};
    const glm::qua<TestType, glm::defaultp> _q2{
        GENERATE(take(1, random(-50.0f, 50.0f))),
        GENERATE(take(1, random(-50.0f, 50.0f))),
        GENERATE(take(1, random(-50.0f, 50.0f))),
        GENERATE(take(1, random(-50.0f, 50.0f))),
    };

    const TQuaternion<TestType> q1(_q1.w, _q1.x, _q1.y, _q1.z);
    const TQuaternion<TestType> q2(_q2.w, _q2.x, _q2.y, _q2.z);

    SECTION("Quaternion multiplication")
    {
        const glm::qua<TestType, glm::defaultp> ExpectedResult = _q1 * _q2;
        const TQuaternion<TestType> Result = q1 * q2;

        CHECK_THAT(Result.w, Catch::Matchers::WithinRel(ExpectedResult.w, Epsilon));
        CHECK_THAT(Result.x, Catch::Matchers::WithinRel(ExpectedResult.x, Epsilon));
        CHECK_THAT(Result.y, Catch::Matchers::WithinRel(ExpectedResult.y, Epsilon));
        CHECK_THAT(Result.z, Catch::Matchers::WithinRel(ExpectedResult.z, Epsilon));
    }

    SECTION("Quaternion inverse")
    {
        const glm::qua<TestType, glm::defaultp> ExpectedResult = glm::inverse(_q1);

        const TQuaternion<TestType> result = q1.Inverse();

        CHECK_THAT(result.w, Catch::Matchers::WithinRel(ExpectedResult.w, Epsilon));
        CHECK_THAT(result.x, Catch::Matchers::WithinRel(ExpectedResult.x, Epsilon));
        CHECK_THAT(result.y, Catch::Matchers::WithinRel(ExpectedResult.y, Epsilon));
        CHECK_THAT(result.z, Catch::Matchers::WithinRel(ExpectedResult.z, Epsilon));
    }

    SECTION("Quaternion normalization")
    {
        const glm::qua<TestType, glm::defaultp> ExpectedResult = glm::normalize(_q1);

        const TQuaternion<TestType> result = q1.Normalize();

        CHECK_THAT(result.w, Catch::Matchers::WithinRel(ExpectedResult.w, Epsilon));
        CHECK_THAT(result.x, Catch::Matchers::WithinRel(ExpectedResult.x, Epsilon));
        CHECK_THAT(result.y, Catch::Matchers::WithinRel(ExpectedResult.y, Epsilon));
        CHECK_THAT(result.z, Catch::Matchers::WithinRel(ExpectedResult.z, Epsilon));
    }

    SECTION("Quaternion dot product")
    {
        const TestType ExpectedResult = glm::dot(_q1, _q2);
        const TestType Result = q1.Dot(q2);

        CHECK_THAT(Result, Catch::Matchers::WithinRel(ExpectedResult, Epsilon));
    }

    SECTION("Quaternion equality")
    {
        const glm::qua<TestType, glm::defaultp> _q3 = _q2;

        const TQuaternion<TestType> q3 = q2;

        REQUIRE(q3.w == _q3.w);
        REQUIRE(q3.x == _q3.x);
        REQUIRE(q3.y == _q3.y);
        REQUIRE(q3.z == _q3.z);
    }

    SECTION("Quaternion rotation matrix")
    {
        const glm::mat<4, 4, TestType> ExpectedResult = glm::toMat4(_q1);

        const TMatrix4<TestType> Result = q1.GetRotationMatrix();

        INFO("Quaternion: " << glm::to_string(_q1));
        INFO("Expected: " << glm::to_string(ExpectedResult));

        INFO("Quaternion: " << q1);
        INFO("Result: " << Result);
        for (unsigned i = 0; i < 4; i++)
        {
            for (unsigned j = 0; j < 4; j++)
            {
                INFO("Result[" << i << "][" << j << "]: " << Result[i][j]);
                INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedResult[i][j]);
                CHECK_THAT(Result[i][j], Catch::Matchers::WithinRel(ExpectedResult[i][j], Epsilon));
            }
        }
    }
}

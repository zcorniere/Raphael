#include "Engine/Raphael.hxx"

#include "Engine/Math/Quaternion.hxx"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Quaternion operations")
{
    Catch::StringMaker<float>::precision = 25;
    Catch::StringMaker<double>::precision = 25;

    const glm::quat _q1{GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                        GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f)))};
    const glm::quat _q2{
        GENERATE(take(1, random(-50.0f, 50.0f))),
        GENERATE(take(1, random(-50.0f, 50.0f))),
        GENERATE(take(1, random(-50.0f, 50.0f))),
        GENERATE(take(1, random(-50.0f, 50.0f))),
    };

    const FQuaternion q1(_q1.w, _q1.x, _q1.y, _q1.z);
    const FQuaternion q2(_q2.w, _q2.x, _q2.y, _q2.z);

    SECTION("Quaternion multiplication")
    {
        const glm::quat ExpectedResult = _q1 * _q2;
        const FQuaternion Result = q1 * q2;

        CHECK_THAT(Result.w, Catch::Matchers::WithinRel(ExpectedResult.w));
        CHECK_THAT(Result.x, Catch::Matchers::WithinRel(ExpectedResult.x));
        CHECK_THAT(Result.y, Catch::Matchers::WithinRel(ExpectedResult.y));
        CHECK_THAT(Result.z, Catch::Matchers::WithinRel(ExpectedResult.z));
    }

    SECTION("Quaternion inverse")
    {
        const glm::quat ExpectedResult = glm::inverse(_q1);

        const FQuaternion result = q1.Inverse();

        CHECK_THAT(result.w, Catch::Matchers::WithinRel(ExpectedResult.w));
        CHECK_THAT(result.x, Catch::Matchers::WithinRel(ExpectedResult.x));
        CHECK_THAT(result.y, Catch::Matchers::WithinRel(ExpectedResult.y));
        CHECK_THAT(result.z, Catch::Matchers::WithinRel(ExpectedResult.z));
    }

    SECTION("Quaternion normalization")
    {
        const glm::quat ExpectedResult = glm::normalize(_q1);

        const FQuaternion result = q1.Normalize();

        CHECK_THAT(result.w, Catch::Matchers::WithinRel(ExpectedResult.w));
        CHECK_THAT(result.x, Catch::Matchers::WithinRel(ExpectedResult.x));
        CHECK_THAT(result.y, Catch::Matchers::WithinRel(ExpectedResult.y));
        CHECK_THAT(result.z, Catch::Matchers::WithinRel(ExpectedResult.z));
    }

    SECTION("Quaternion dot product")
    {
        const float ExpectedResult = glm::dot(_q1, _q2);
        const float Result = q1.Dot(q2);

        CHECK_THAT(Result, Catch::Matchers::WithinRel(ExpectedResult));
    }

    SECTION("Quaternion equality")
    {
        const glm::quat _q3 = _q2;

        const FQuaternion q3 = q2;

        REQUIRE(q3.w == _q3.w);
        REQUIRE(q3.x == _q3.x);
        REQUIRE(q3.y == _q3.y);
        REQUIRE(q3.z == _q3.z);
    }

    SECTION("Quaternion rotation matrix")
    {
        const glm::mat4 ExpectedResult = glm::toMat4(_q1);

        const FMatrix4 Result = q1.GetRotationMatrix();

        INFO("Quaternion: " << glm::to_string(_q1));
        INFO("Expected: " << glm::to_string(ExpectedResult));

        INFO("Quaternion: " << q1);
        INFO("Result: " << Result);
        for (unsigned i = 0; i < 4; i++) {
            for (unsigned j = 0; j < 4; j++) {
                INFO("Result[" << i << "][" << j << "]: " << Result[i][j]);
                INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedResult[i][j]);
                CHECK_THAT(Result[i][j], Catch::Matchers::WithinRel(ExpectedResult[i][j]));
            }
        }
    }
}

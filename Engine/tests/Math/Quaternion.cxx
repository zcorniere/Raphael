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
#include <iostream>

TEST_CASE("Quaternion operations")
{
    Catch::StringMaker<float>::precision = 25;
    Catch::StringMaker<double>::precision = 25;

    SECTION("Quaternion multiplication")
    {
        const glm::quat _q1{1.0f, 2.0f, 3.0f, 4.0f};
        const glm::quat _q2{5.0f, 6.0f, 7.0f, 8.0f};
        const glm::quat ExpectedResult = _q1 * _q2;

        const FQuaternion q1(1.0f, 2.0f, 3.0f, 4.0f);
        const FQuaternion q2(5.0f, 6.0f, 7.0f, 8.0f);
        const FQuaternion Result = q1 * q2;

        CHECK_THAT(Result.w, Catch::Matchers::WithinRel(ExpectedResult.w));
        CHECK_THAT(Result.x, Catch::Matchers::WithinRel(ExpectedResult.x));
        CHECK_THAT(Result.y, Catch::Matchers::WithinRel(ExpectedResult.y));
        CHECK_THAT(Result.z, Catch::Matchers::WithinRel(ExpectedResult.z));
    }

    SECTION("Quaternion inverse")
    {
        const glm::quat _q{1.0f, 2.0f, 3.0f, 4.0f};
        const glm::quat ExpectedResult = glm::inverse(_q);

        const FQuaternion q{1.0f, 2.0f, 3.0f, 4.0f};
        const FQuaternion result = q.Inverse();

        CHECK_THAT(result.w, Catch::Matchers::WithinRel(ExpectedResult.w));
        CHECK_THAT(result.x, Catch::Matchers::WithinRel(ExpectedResult.x));
        CHECK_THAT(result.y, Catch::Matchers::WithinRel(ExpectedResult.y));
        CHECK_THAT(result.z, Catch::Matchers::WithinRel(ExpectedResult.z));
    }

    SECTION("Quaternion normalization")
    {
        const glm::quat _q{1.0f, 2.0f, 3.0f, 4.0f};
        const glm::quat ExpectedResult = glm::normalize(_q);

        const FQuaternion q{1.0f, 2.0f, 3.0f, 4.0f};
        const FQuaternion result = q.Normalize();

        CHECK_THAT(result.w, Catch::Matchers::WithinRel(ExpectedResult.w));
        CHECK_THAT(result.x, Catch::Matchers::WithinRel(ExpectedResult.x));
        CHECK_THAT(result.y, Catch::Matchers::WithinRel(ExpectedResult.y));
        CHECK_THAT(result.z, Catch::Matchers::WithinRel(ExpectedResult.z));
    }

    SECTION("Quaternion dot product")
    {
        const glm::quat _q1{1.0f, 2.0f, 3.0f, 4.0f};
        const glm::quat _q2{5.0f, 6.0f, 7.0f, 8.0f};
        const float ExpectedResult = glm::dot(_q1, _q2);

        const FQuaternion q1{1.0f, 2.0f, 3.0f, 4.0f};
        const FQuaternion q2{5.0f, 6.0f, 7.0f, 8.0f};
        const float Result = q1.Dot(q2);

        CHECK_THAT(Result, Catch::Matchers::WithinRel(ExpectedResult));
    }

    SECTION("Quaternion equality")
    {
        const glm::quat _q1{1.0f, 2.0f, 3.0f, 4.0f};
        const glm::quat _q2{1.0f, 2.0f, 3.0f, 4.0f};

        const FQuaternion q1{1.0f, 2.0f, 3.0f, 4.0f};
        const FQuaternion q2{1.0f, 2.0f, 3.0f, 4.0f};

        REQUIRE(q1 == q2);
    }

    SECTION("Quaternion rotation matrix")
    {
        const glm::quat _q{1.0f, 2.0f, 3.0f, 4.0f};
        const glm::mat4 ExpectedResult = glm::toMat4(_q);

        const FQuaternion q{1.0f, 2.0f, 3.0f, 4.0f};
        const FMatrix4 Result = q.GetRotationMatrix();

        INFO("Quaternion: " << glm::to_string(_q));
        INFO("Expected: " << glm::to_string(ExpectedResult));

        INFO("Quaternion: " << q);
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

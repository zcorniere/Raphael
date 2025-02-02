#include "Engine/Raphael.hxx"

#include "Engine/Math/Math.hxx"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Degrees and Radians convertion")
{
    SECTION("Degree to Radian")
    {
        const float Value = GENERATE(take(10, random(-50.0f, 50.0f)));
        const float Result = Math::DegreeToRadian(Value);
        const float ExpectedResult = glm::radians(Value);

        CHECK_THAT(Result, Catch::Matchers::WithinRel(ExpectedResult));
    }

    SECTION("Radian to Degree")
    {
        const float Value = GENERATE(take(10, random(-50.0f, 50.0f)));
        const float Result = Math::RadianToDegree(Value);
        const float ExpectedResult = glm::degrees(Value);

        CHECK_THAT(Result, Catch::Matchers::WithinRel(ExpectedResult));
    }
}

TEST_CASE("Dot product")
{
    SECTION("Dot product 2D")
    {
        const glm::vec2 _v1{GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f)))};
        const glm::vec2 _v2{GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f)))};

        const FVector2 v1(_v1.x, _v1.y);
        const FVector2 v2(_v2.x, _v2.y);

        const float Result = Math::Dot(v1, v2);
        const float ExpectedResult = glm::dot(_v1, _v2);

        CHECK_THAT(Result, Catch::Matchers::WithinRel(ExpectedResult));
    }

    SECTION("Dot product 3D")
    {
        const glm::vec3 _v1{GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                            GENERATE(take(1, random(-50.0f, 50.0f)))};
        const glm::vec3 _v2{GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                            GENERATE(take(1, random(-50.0f, 50.0f)))};

        const FVector3 v1(_v1.x, _v1.y, _v1.z);
        const FVector3 v2(_v2.x, _v2.y, _v2.z);

        const float Result = Math::Dot(v1, v2);
        const float ExpectedResult = glm::dot(_v1, _v2);

        CHECK_THAT(Result, Catch::Matchers::WithinRel(ExpectedResult));
    }
}

TEST_CASE("Normalize")
{
    SECTION("Normalize 2D")
    {
        const glm::vec2 _v1{GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f)))};

        const FVector2 v1(_v1.x, _v1.y);

        const FVector2 Result = Math::Normalize(v1);
        const glm::vec2 ExpectedResult = glm::normalize(_v1);

        CHECK_THAT(Result.x, Catch::Matchers::WithinRel(ExpectedResult.x));
        CHECK_THAT(Result.y, Catch::Matchers::WithinRel(ExpectedResult.y));
    }

    SECTION("Normalize 3D")
    {
        const glm::vec3 _v1{GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                            GENERATE(take(1, random(-50.0f, 50.0f)))};

        const FVector3 v1(_v1.x, _v1.y, _v1.z);

        const FVector3 Result = Math::Normalize(v1);
        const glm::vec3 ExpectedResult = glm::normalize(_v1);

        CHECK_THAT(Result.x, Catch::Matchers::WithinRel(ExpectedResult.x));
        CHECK_THAT(Result.y, Catch::Matchers::WithinRel(ExpectedResult.y));
        CHECK_THAT(Result.z, Catch::Matchers::WithinRel(ExpectedResult.z));
    }

    SECTION("Normalize 4D")
    {
        const glm::vec4 _v1{GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                            GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f)))};

        const FVector4 v1(_v1.x, _v1.y, _v1.z, _v1.w);

        const FVector4 Result = Math::Normalize(v1);
        const glm::vec4 ExpectedResult = glm::normalize(_v1);

        CHECK_THAT(Result.x, Catch::Matchers::WithinRel(ExpectedResult.x));
        CHECK_THAT(Result.y, Catch::Matchers::WithinRel(ExpectedResult.y));
        CHECK_THAT(Result.z, Catch::Matchers::WithinRel(ExpectedResult.z));
        CHECK_THAT(Result.w, Catch::Matchers::WithinRel(ExpectedResult.w));
    }
}

TEST_CASE("Cross product")
{
    const glm::vec3 _v1{GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                        GENERATE(take(1, random(-50.0f, 50.0f)))};
    const glm::vec3 _v2{GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                        GENERATE(take(1, random(-50.0f, 50.0f)))};

    const FVector3 v1(_v1.x, _v1.y, _v1.z);
    const FVector3 v2(_v2.x, _v2.y, _v2.z);

    const FVector3 Result = Math::Cross(v1, v2);
    const glm::vec3 ExpectedResult = glm::cross(_v1, _v2);

    CHECK_THAT(Result.x, Catch::Matchers::WithinRel(ExpectedResult.x));
    CHECK_THAT(Result.y, Catch::Matchers::WithinRel(ExpectedResult.y));
    CHECK_THAT(Result.z, Catch::Matchers::WithinRel(ExpectedResult.z));
}

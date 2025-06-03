#include "Engine/Raphael.hxx"

#include "Engine/Math/Transform.hxx"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Transform Matrices")
{
    const glm::vec3 Location{GENERATE(take(2, random(-50.0f, 50.0f))), GENERATE(take(2, random(-50.0f, 50.0f))),
                             GENERATE(take(2, random(-50.0f, 50.0f)))};
    const glm::quat Rotation{GENERATE(take(2, random(-50.0f, 50.0f))), GENERATE(take(2, random(-50.0f, 50.0f))),
                             GENERATE(take(2, random(-50.0f, 50.0f))), GENERATE(take(2, random(-50.0f, 50.0f)))};
    const glm::vec3 Scale{GENERATE(take(2, random(-50.0f, 50.0f))), GENERATE(take(2, random(-50.0f, 50.0f))),
                          GENERATE(take(2, random(-50.0f, 50.0f)))};

    FTransform Transform(FVector3{Location.x, Location.y, Location.z},
                         FQuaternion(Rotation.w, Rotation.x, Rotation.y, Rotation.z),
                         FVector3{Scale.x, Scale.y, Scale.z});

    SECTION("Translation Matrix")
    {
        const glm::mat4 ExpectedTranslationMatrix = glm::translate(glm::mat4(1.0f), Location);
        const FMatrix4 TranslationMatrix = Transform.GetTranslationMatrix();

        INFO("ExpectedTranslationMatrix: " << glm::to_string(ExpectedTranslationMatrix));
        INFO("TranslationMatrix: " << TranslationMatrix);
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                INFO("Result[" << i << "][" << j << "]: " << TranslationMatrix[i][j]);
                INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedTranslationMatrix[i][j]);
                CHECK_THAT(TranslationMatrix[i][j], Catch::Matchers::WithinRel(ExpectedTranslationMatrix[i][j]));
            }
        }
    }

    SECTION("Rotation Matrix")
    {
        const glm::mat4 ExpectedRotationMatrix = glm::toMat4(Rotation);
        const FMatrix4 RotationMatrix = Transform.GetRotationMatrix();

        INFO("ExpectedRotationMatrix: " << glm::to_string(ExpectedRotationMatrix));
        INFO("RotationMatrix: " << RotationMatrix);
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                INFO("Result[" << i << "][" << j << "]: " << RotationMatrix[i][j]);
                INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedRotationMatrix[i][j]);
                CHECK_THAT(RotationMatrix[i][j], Catch::Matchers::WithinRel(ExpectedRotationMatrix[i][j]));
            }
        }
    }

    SECTION("Scale Matrix")
    {
        const glm::mat4 ExpectedScaleMatrix = glm::scale(glm::mat4(1.0f), Scale);
        const FMatrix4 ScaleMatrix = Transform.GetScaleMatrix();

        INFO("ExpectedScaleMatrix: " << glm::to_string(ExpectedScaleMatrix));
        INFO("ScaleMatrix: " << ScaleMatrix);
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                INFO("Result[" << i << "][" << j << "]: " << ScaleMatrix[i][j]);
                INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedScaleMatrix[i][j]);
                CHECK_THAT(ScaleMatrix[i][j], Catch::Matchers::WithinRel(ExpectedScaleMatrix[i][j]));
            }
        }
    }

    SECTION("Model Matrix")
    {
        const glm::mat4 ExpectedTransformMatrix = glm::translate(glm::mat4(1.0f), Location);
        const glm::mat4 ExpectedRotationMatrix = glm::toMat4(Rotation);
        const glm::mat4 ExpectedScaleMatrix = glm::scale(glm::mat4(1.0f), Scale);

        const FMatrix4 TransformMatrix = Transform.GetTranslationMatrix();
        const FMatrix4 RotationMatrix = Transform.GetRotationMatrix();
        const FMatrix4 ScaleMatrix = Transform.GetScaleMatrix();

        const glm::mat4 ExpectedModelMatrix = ExpectedTransformMatrix * ExpectedRotationMatrix * ExpectedScaleMatrix;
        const FMatrix4 ModelMatrix = Transform.GetModelMatrix();

        CHECK(ModelMatrix == TransformMatrix * RotationMatrix * ScaleMatrix);

        INFO("ExpectedModelMatrix: " << glm::to_string(ExpectedModelMatrix));
        INFO("ModelMatrix: " << ModelMatrix);
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                INFO("Result[" << i << "][" << j << "]: " << ModelMatrix[i][j]);
                INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedModelMatrix[i][j]);
                CHECK_THAT(ModelMatrix[i][j], Catch::Matchers::WithinRel(ExpectedModelMatrix[i][j]));
            }
        }
    }
}

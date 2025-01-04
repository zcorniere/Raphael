#include "Engine/Raphael.hxx"

#include "Engine/Math/Transform.hxx"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Transform Matrices")
{
    glm::vec3 Location{0.0f, 0.0f, -2.0f};
    glm::quat Rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 Scale{1.0f, 1.0f, 1.0f};

    FTransform Transform(FVector3{0.0f, 0.0f, -2.0f}, FQuaternion(), FVector3{1.0f, 1.0f, 1.0f});

    SECTION("Translation Matrix")
    {
        glm::mat4 ExpectedTranslationMatrix = glm::translate(glm::mat4(1.0f), Location);
        FMatrix4 TranslationMatrix = Transform.GetTranslationMatrix();

        INFO("ExpectedTranslationMatrix: " << glm::to_string(ExpectedTranslationMatrix));
        INFO("TranslationMatrix: " << TranslationMatrix);
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                INFO("Result[" << i << "][" << j << "]: " << TranslationMatrix[i][j]);
                INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedTranslationMatrix[i][j]);
                CHECK(TranslationMatrix[i][j] == ExpectedTranslationMatrix[i][j]);
            }
        }
    }

    SECTION("Rotation Matrix")
    {
        glm::mat4 ExpectedRotationMatrix = glm::toMat4(Rotation);
        FMatrix4 RotationMatrix = Transform.GetRotationMatrix();

        INFO("ExpectedRotationMatrix: " << glm::to_string(ExpectedRotationMatrix));
        INFO("RotationMatrix: " << RotationMatrix);
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                INFO("Result[" << i << "][" << j << "]: " << RotationMatrix[i][j]);
                INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedRotationMatrix[i][j]);
                CHECK(RotationMatrix[i][j] == ExpectedRotationMatrix[i][j]);
            }
        }
    }

    SECTION("Scale Matrix")
    {
        glm::mat4 ExpectedScaleMatrix = glm::scale(glm::mat4(1.0f), Scale);
        FMatrix4 ScaleMatrix = Transform.GetScaleMatrix();

        INFO("ExpectedScaleMatrix: " << glm::to_string(ExpectedScaleMatrix));
        INFO("ScaleMatrix: " << ScaleMatrix);
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                INFO("Result[" << i << "][" << j << "]: " << ScaleMatrix[i][j]);
                INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedScaleMatrix[i][j]);
                CHECK(ScaleMatrix[i][j] == ExpectedScaleMatrix[i][j]);
            }
        }
    }

    SECTION("Model Matrix")
    {
        glm::mat4 ExpectedModelMatrix =
            glm::translate(glm::mat4(1.0f), Location) * glm::toMat4(Rotation) * glm::scale(glm::mat4(1.0f), Scale);
        FMatrix4 ModelMatrix = Transform.GetModelMatrix();

        INFO("ExpectedModelMatrix: " << glm::to_string(ExpectedModelMatrix));
        INFO("ModelMatrix: " << ModelMatrix);
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                INFO("Result[" << i << "][" << j << "]: " << ModelMatrix[i][j]);
                INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedModelMatrix[i][j]);
                CHECK(ModelMatrix[i][j] == ExpectedModelMatrix[i][j]);
            }
        }
    }
}

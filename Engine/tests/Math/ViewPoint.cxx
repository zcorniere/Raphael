#include "Engine/Raphael.hxx"

#include "Engine/Math/ViewPoint.hxx"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Viewpoint Tests")
{
    Math::TViewPoint<float> Viewpoint(45.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    Viewpoint.SetLocation({10.f, 0.f, -1.f});

    SECTION("Projection Matrix")
    {
        FMatrix4 ProjectionMatrix = Viewpoint.GetProjectionMatrix();

        glm::mat4 ExpectedProjectionMatrix = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 100.0f);

        INFO("ProjectionMatrix: " << ProjectionMatrix);
        INFO("ExpectedProjectionMatrix: " << glm::to_string(ExpectedProjectionMatrix));
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                INFO("Result[" << i << "][" << j << "]: " << ProjectionMatrix[i][j]);
                INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedProjectionMatrix[i][j]);
                CHECK(ProjectionMatrix[i][j] == ExpectedProjectionMatrix[i][j]);
            }
        }
    }

    // SECTION("View Matrix")
    // {
    //     FMatrix4 ViewMatrix = Viewpoint.GetViewMatrix();

    //     glm::vec3 Location = glm::vec3(10.f, 0.f, -1.f);
    //     // Camera at (0, 0, 5)
    //     glm::quat orientation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));    // Rotate 180° around Y

    //     // Calculate forward and up vectors from quaternion
    //     glm::vec3 forward = glm::normalize(orientation * glm::vec3(0, 1, 0));
    //     glm::vec3 up = glm::normalize(orientation * glm::vec3(0, 0, 1));

    //     // Calculate target for glm::lookAt
    //     glm::vec3 target = Location + forward;
    //     glm::mat4 ExpectedViewMatrix = glm::lookAt(Location, target, up);

    //     INFO("ViewMatrix: " << ViewMatrix);
    //     INFO("ExpectedViewMatrix: " << glm::to_string(ExpectedViewMatrix));
    //     for (int i = 0; i < 4; i++) {
    //         for (int j = 0; j < 4; j++) {
    //             INFO("Result[" << i << "][" << j << "]: " << ViewMatrix[i][j]);
    //             INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedViewMatrix[i][j]);
    //             CHECK(ViewMatrix[i][j] == ExpectedViewMatrix[i][j]);
    //         }
    //     }
    // }

    // SECTION("ViewProjmatrix")
    // {
    //     FMatrix4 ViewProjMatrix = Viewpoint.GetViewProjectionMatrix();

    //     glm::vec3 Location = glm::vec3(10.f, 0.f, -1.f);
    //     glm::vec3 Center = {Location.x + Math::FrontVector.x, Location.y + Math::FrontVector.y,
    //                         Location.z + Math::FrontVector.z};
    //     glm::vec3 Up = {Math::UpVector.x, Math::UpVector.y, Math::UpVector.z};
    //     glm::mat4 ExpectedViewProjMatrix =
    //         glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 100.0f) * glm::lookAt(Location, Center, Up);

    //     INFO("ViewProjMatrix: " << ViewProjMatrix);
    //     INFO("ExpectedViewProjMatrix: " << glm::to_string(ExpectedViewProjMatrix));
    //     for (int i = 0; i < 4; i++) {
    //         for (int j = 0; j < 4; j++) {
    //             INFO("Result[" << i << "][" << j << "]: " << ViewProjMatrix[i][j]);
    //             INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedViewProjMatrix[i][j]);
    //             CHECK_THAT(ViewProjMatrix[i][j], Catch::Matchers::WithinRel(ExpectedViewProjMatrix[i][j]));
    //         }
    //     }
    // }
}

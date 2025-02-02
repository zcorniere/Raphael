#include "Engine/Raphael.hxx"

#include "Engine/Math/ViewPoint.hxx"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Viewpoint Tests")
{
    const FVector3 Location(GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                            GENERATE(take(1, random(-50.0f, 50.0f))));
    const float fAspectRatio = 16.0f / 9.0f;
    const float fFOV = GENERATE(take(1, random(0.0f, 180.0f)));
    const float fFar = 100.0f;
    const float fNear = 0.1f;

    Math::TViewPoint<float> Viewpoint(fFOV, fNear, fFar, fAspectRatio);
    Viewpoint.SetLocation(Location);

    SECTION("Projection Matrix")
    {
        FMatrix4 ProjectionMatrix = Viewpoint.GetProjectionMatrix();

        glm::mat4 ExpectedProjectionMatrix = glm::perspective(glm::radians(fFOV), fAspectRatio, fNear, fFar);

        INFO("ProjectionMatrix: " << ProjectionMatrix);
        INFO("ExpectedProjectionMatrix: " << glm::to_string(ExpectedProjectionMatrix));
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                INFO("Result[" << i << "][" << j << "]: " << ProjectionMatrix[i][j]);
                INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedProjectionMatrix[i][j]);
                CHECK_THAT(ProjectionMatrix[i][j], Catch::Matchers::WithinRel(ExpectedProjectionMatrix[i][j]));
            }
        }
    }

    // SECTION("View Matrix")
    // {
    //     FMatrix4 ViewMatrix = Viewpoint.GetViewMatrix();

    //     glm::vec3 ExpectedLocation(Location.x, Location.y, Location.z);
    //     glm::vec3 Center = {ExpectedLocation.x + Math::FrontVector.x, ExpectedLocation.y + Math::FrontVector.y,
    //                         ExpectedLocation.z + Math::FrontVector.z};
    //     glm::vec3 Up = {Math::UpVector.x, Math::UpVector.y, Math::UpVector.z};
    //     glm::mat4 ExpectedViewMatrix = glm::lookAt(ExpectedLocation, Center, Up);

    //     INFO("ViewMatrix: " << ViewMatrix);
    //     INFO("ExpectedViewMatrix: " << glm::to_string(ExpectedViewMatrix));
    //     for (int i = 0; i < 4; i++) {
    //         for (int j = 0; j < 4; j++) {
    //             INFO("Result[" << i << "][" << j << "]: " << ViewMatrix[i][j]);
    //             INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedViewMatrix[i][j]);
    //             CHECK_THAT(ViewMatrix[i][j], Catch::Matchers::WithinRel(ExpectedViewMatrix[i][j]));
    //         }
    //     }
    // }

    // SECTION("ViewProjmatrix")
    // {
    //     FMatrix4 ViewProjMatrix = Viewpoint.GetViewProjectionMatrix();

    //     glm::vec3 ExpectedLocation(Location.x, Location.y, Location.z);
    //     glm::vec3 Center = {ExpectedLocation.x + Math::FrontVector.x, ExpectedLocation.y + Math::FrontVector.y,
    //                         ExpectedLocation.z + Math::FrontVector.z};
    //     glm::vec3 Up = {Math::UpVector.x, Math::UpVector.y, Math::UpVector.z};
    //     glm::mat4 ExpectedViewProjMatrix =
    //         glm::perspective(glm::radians(fFOV), fAspectRatio, fNear, fFar) * glm::lookAt(ExpectedLocation, Center,
    //         Up);

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

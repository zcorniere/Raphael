#include "Engine/Raphael.hxx"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Matrix")
{
    FMatrix4 Matrix = FMatrix4::Identity();
    glm::mat4 ExpectedMatrix = glm::mat4(1.0f);

    Matrix[0][0] = 1;
    ExpectedMatrix[0][0] = 1;
    Matrix[1][0] = 3;
    ExpectedMatrix[1][0] = 3;
    Matrix[2][0] = 5;
    ExpectedMatrix[2][0] = 5;
    Matrix[3][0] = 3;
    ExpectedMatrix[3][0] = 3;

    Matrix[0][1] = 2;
    ExpectedMatrix[0][1] = 2;
    Matrix[1][1] = 4;
    ExpectedMatrix[1][1] = 4;

    SECTION("Matrix multiplication")
    {
        FMatrix4 Result = Matrix * Matrix;
        glm::mat4 ExpectedResult = ExpectedMatrix * ExpectedMatrix;

        INFO("Result: " << Result);
        INFO("ExpectedResult: " << glm::to_string(ExpectedResult));
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                INFO("Result[" << i << "][" << j << "]: " << Result[i][j]);
                INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedResult[i][j]);
                CHECK(Result[i][j] == ExpectedResult[i][j]);
            }
        }
    }
}

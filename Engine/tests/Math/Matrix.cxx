#include "Engine/Raphael.hxx"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

TEST_CASE("Matrix")
{
    // Create a random matrix
    const glm::mat4 ExpectedMatrix(GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                                   GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                                   GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                                   GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                                   GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                                   GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                                   GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                                   GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))));
    const FMatrix4 Matrix(ExpectedMatrix[0][0], ExpectedMatrix[0][1], ExpectedMatrix[0][2], ExpectedMatrix[0][3],
                          ExpectedMatrix[1][0], ExpectedMatrix[1][1], ExpectedMatrix[1][2], ExpectedMatrix[1][3],
                          ExpectedMatrix[2][0], ExpectedMatrix[2][1], ExpectedMatrix[2][2], ExpectedMatrix[2][3],
                          ExpectedMatrix[3][0], ExpectedMatrix[3][1], ExpectedMatrix[3][2], ExpectedMatrix[3][3]);

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

    SECTION("Matrix addition")
    {
        FMatrix4 Result = Matrix + Matrix;
        glm::mat4 ExpectedResult = ExpectedMatrix + ExpectedMatrix;

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
    SECTION("Matrix substraction")
    {
        FMatrix4 Result = Matrix - Matrix;
        glm::mat4 ExpectedResult = ExpectedMatrix - ExpectedMatrix;

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

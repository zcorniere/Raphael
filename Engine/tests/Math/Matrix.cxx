#include "Engine/Raphael.hxx"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "Common.hxx"

TEMPLATE_TEST_CASE("Matrix", "[Math][Quaternion]", float, double)
{
    const TestType Epsilon = TEpsilon<TestType>::Value;

    // Create a random matrix
    const glm::mat<4, 4, TestType> ExpectedMatrix(
        GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
        GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
        GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
        GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
        GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
        GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
        GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
        GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))));
    const TMatrix4<TestType> Matrix(
        ExpectedMatrix[0][0], ExpectedMatrix[0][1], ExpectedMatrix[0][2], ExpectedMatrix[0][3], ExpectedMatrix[1][0],
        ExpectedMatrix[1][1], ExpectedMatrix[1][2], ExpectedMatrix[1][3], ExpectedMatrix[2][0], ExpectedMatrix[2][1],
        ExpectedMatrix[2][2], ExpectedMatrix[2][3], ExpectedMatrix[3][0], ExpectedMatrix[3][1], ExpectedMatrix[3][2],
        ExpectedMatrix[3][3]);

    SECTION("Matrix multiplication")
    {
        TMatrix4<TestType> Result = Matrix * Matrix;
        glm::mat<4, 4, TestType> ExpectedResult = ExpectedMatrix * ExpectedMatrix;

        INFO("Result: " << Result);
        INFO("ExpectedResult: " << glm::to_string(ExpectedResult));
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                INFO("Result[" << i << "][" << j << "]: " << Result[i][j]);
                INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedResult[i][j]);
                CHECK_THAT(Result[i][j], Catch::Matchers::WithinRel(ExpectedResult[i][j], Epsilon));
            }
        }
    }

    SECTION("Matrix addition")
    {
        TMatrix4<TestType> Result = Matrix + Matrix;
        glm::mat<4, 4, TestType> ExpectedResult = ExpectedMatrix + ExpectedMatrix;

        INFO("Result: " << Result);
        INFO("ExpectedResult: " << glm::to_string(ExpectedResult));
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                INFO("Result[" << i << "][" << j << "]: " << Result[i][j]);
                INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedResult[i][j]);
                CHECK_THAT(Result[i][j], Catch::Matchers::WithinRel(ExpectedResult[i][j], Epsilon));
            }
        }
    }
    SECTION("Matrix substraction")
    {
        TMatrix4<TestType> Result = Matrix - Matrix;
        glm::mat<4, 4, TestType> ExpectedResult = ExpectedMatrix - ExpectedMatrix;

        INFO("Result: " << Result);
        INFO("ExpectedResult: " << glm::to_string(ExpectedResult));
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                INFO("Result[" << i << "][" << j << "]: " << Result[i][j]);
                INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedResult[i][j]);
                CHECK_THAT(Result[i][j], Catch::Matchers::WithinRel(ExpectedResult[i][j], Epsilon));
            }
        }
    }
}

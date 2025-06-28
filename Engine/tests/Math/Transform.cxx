#include "Engine/Raphael.hxx"

#include "Engine/Math/Transform.hxx"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "Common.hxx"

TEMPLATE_TEST_CASE("Transform Matrices", "[Math][Transform]", float, double)
{
    const TestType Epsilon = TEpsilon<TestType>::Value;
    const glm::vec<3, TestType> Location{GENERATE(take(1, random(-50.0f, 50.0f))),
                                         GENERATE(take(1, random(-50.0f, 50.0f))),
                                         GENERATE(take(1, random(-50.0f, 50.0f)))};
    const glm::qua<TestType, glm::defaultp> Rotation{
        GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
        GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f)))};
    const glm::vec<3, TestType> Scale{GENERATE(take(1, random(-50.0f, 50.0f))),
                                      GENERATE(take(1, random(-50.0f, 50.0f))),
                                      GENERATE(take(1, random(-50.0f, 50.0f)))};

    TTransform<TestType> Transform(TVector3<TestType>{Location.x, Location.y, Location.z},
                                   TQuaternion<TestType>(Rotation.w, Rotation.x, Rotation.y, Rotation.z),
                                   TVector3<TestType>{Scale.x, Scale.y, Scale.z});

    SECTION("Translation Matrix")
    {
        const glm::mat<4, 4, TestType> ExpectedTranslationMatrix =
            glm::translate(glm::mat<4, 4, TestType>(1.0f), Location);
        const TMatrix4<TestType> TranslationMatrix = Transform.GetTranslationMatrix();

        INFO("ExpectedTranslationMatrix: " << glm::to_string(ExpectedTranslationMatrix));
        INFO("TranslationMatrix: " << TranslationMatrix);
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                INFO("Result[" << i << "][" << j << "]: " << TranslationMatrix[i][j]);
                INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedTranslationMatrix[i][j]);
                CHECK_THAT(TranslationMatrix[i][j],
                           Catch::Matchers::WithinRel(ExpectedTranslationMatrix[i][j], Epsilon));
            }
        }
    }

    SECTION("Rotation Matrix")
    {
        const glm::mat<4, 4, TestType> ExpectedRotationMatrix = glm::toMat4(Rotation);
        const TMatrix4<TestType> RotationMatrix = Transform.GetRotationMatrix();

        INFO("ExpectedRotationMatrix: " << glm::to_string(ExpectedRotationMatrix));
        INFO("RotationMatrix: " << RotationMatrix);
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                INFO("Result[" << i << "][" << j << "]: " << RotationMatrix[i][j]);
                INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedRotationMatrix[i][j]);
                CHECK_THAT(RotationMatrix[i][j], Catch::Matchers::WithinRel(ExpectedRotationMatrix[i][j], Epsilon));
            }
        }
    }

    SECTION("Scale Matrix")
    {
        const glm::mat<4, 4, TestType> ExpectedScaleMatrix = glm::scale(glm::mat<4, 4, TestType>(1.0f), Scale);
        const TMatrix4<TestType> ScaleMatrix = Transform.GetScaleMatrix();

        INFO("ExpectedScaleMatrix: " << glm::to_string(ExpectedScaleMatrix));
        INFO("ScaleMatrix: " << ScaleMatrix);
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                INFO("Result[" << i << "][" << j << "]: " << ScaleMatrix[i][j]);
                INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedScaleMatrix[i][j]);
                CHECK_THAT(ScaleMatrix[i][j], Catch::Matchers::WithinRel(ExpectedScaleMatrix[i][j], Epsilon));
            }
        }
    }

    SECTION("Model Matrix")
    {
        const glm::mat<4, 4, TestType> ExpectedTransformMatrix =
            glm::translate(glm::mat<4, 4, TestType>(1.0f), Location);
        const glm::mat<4, 4, TestType> ExpectedRotationMatrix = glm::toMat4(Rotation);
        const glm::mat<4, 4, TestType> ExpectedScaleMatrix = glm::scale(glm::mat<4, 4, TestType>(1.0f), Scale);

        const TMatrix4<TestType> TransformMatrix = Transform.GetTranslationMatrix();
        const TMatrix4<TestType> RotationMatrix = Transform.GetRotationMatrix();
        const TMatrix4<TestType> ScaleMatrix = Transform.GetScaleMatrix();

        const glm::mat<4, 4, TestType> ExpectedModelMatrix =
            ExpectedTransformMatrix * ExpectedRotationMatrix * ExpectedScaleMatrix;
        const TMatrix4<TestType> ModelMatrix = Transform.GetModelMatrix();

        CHECK(ModelMatrix == TransformMatrix * RotationMatrix * ScaleMatrix);

        INFO("ExpectedModelMatrix: " << glm::to_string(ExpectedModelMatrix));
        INFO("ModelMatrix: " << ModelMatrix);
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                INFO("Result[" << i << "][" << j << "]: " << ModelMatrix[i][j]);
                INFO("ExpectedResult[" << i << "][" << j << "]: " << ExpectedModelMatrix[i][j]);
                CHECK_THAT(ModelMatrix[i][j], Catch::Matchers::WithinRel(ExpectedModelMatrix[i][j], Epsilon));
            }
        }
    }
}

TEMPLATE_TEST_CASE("Transform Matrices Batches", "[Math][Transform]", float, double)
{
    const TestType Epsilon = TEpsilon<TestType>::Value;
    const glm::vec<3, TestType> Location{GENERATE(take(1, random(-50.0f, 50.0f))),
                                         GENERATE(take(1, random(-50.0f, 50.0f))),
                                         GENERATE(take(1, random(-50.0f, 50.0f)))};
    const glm::qua<TestType, glm::defaultp> Rotation{
        GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
        GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f)))};
    const glm::vec<3, TestType> Scale{GENERATE(take(1, random(-50.0f, 50.0f))),
                                      GENERATE(take(1, random(-50.0f, 50.0f))),
                                      GENERATE(take(1, random(-50.0f, 50.0f)))};

    TTransform<TestType> Transform(TVector3<TestType>{Location.x, Location.y, Location.z},
                                   TQuaternion<TestType>(Rotation.w, Rotation.x, Rotation.y, Rotation.z),
                                   TVector3<TestType>{Scale.x, Scale.y, Scale.z});

    const size_t Count = GENERATE(8, 16, 32);
    TArray<TestType, 64> PositionX, PositionY, PositionZ;
    TArray<TestType, 64> QuaternionX, QuaternionY, QuaternionZ, QuaternionW;
    TArray<TestType, 64> ScaleX, ScaleY, ScaleZ;
    TArray<TMatrix4<TestType>, 64> OutModelMatrix(Count);

    PositionX.Resize(Count);
    PositionY.Resize(Count);
    PositionZ.Resize(Count);
    QuaternionX.Resize(Count);
    QuaternionY.Resize(Count);
    QuaternionZ.Resize(Count);
    QuaternionW.Resize(Count);
    ScaleX.Resize(Count);
    ScaleY.Resize(Count);
    ScaleZ.Resize(Count);

    for (size_t i = 0; i < Count; ++i)
    {
        PositionX[i] = Location.x;
        PositionY[i] = Location.y;
        PositionZ[i] = Location.z;
        QuaternionX[i] = Rotation.x;
        QuaternionY[i] = Rotation.y;
        QuaternionZ[i] = Rotation.z;
        QuaternionW[i] = Rotation.w;
        ScaleX[i] = Scale.x;
        ScaleY[i] = Scale.y;
        ScaleZ[i] = Scale.z;
    }

    SECTION("Model Matrix Batch - AVX2")
    {
        if (!FPlatformMisc::GetCPUInformation().AVX2)
        {
            SKIP("AVX2 is not supported on this CPU");
        }

        int WorkedCount = Math::ComputeModelMatrixBatch_AVX2(
            Count, PositionX.Raw(), PositionY.Raw(), PositionZ.Raw(), QuaternionX.Raw(), QuaternionY.Raw(),
            QuaternionZ.Raw(), QuaternionW.Raw(), ScaleX.Raw(), ScaleY.Raw(), ScaleZ.Raw(),
            reinterpret_cast<TestType*>(OutModelMatrix.Raw()));
        REQUIRE(WorkedCount == Count);

        INFO("Count: " << Count);
        for (size_t Index = 0; Index < WorkedCount; Index++)
        {
            INFO("Model matrix: " << Transform.GetModelMatrix());
            INFO("SIMD Model matrix: [" << Index << "]: " << OutModelMatrix[Index]);
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    INFO("Result[" << Index << "[" << i << "][" << j << "]: " << OutModelMatrix[Index][i][j]);
                    INFO("ExpectedResult[" << i << "][" << j << "]: " << Transform.GetModelMatrix()[i][j]);
                    CHECK_THAT(OutModelMatrix[Index][i][j],
                               Catch::Matchers::WithinRel(Transform.GetModelMatrix()[i][j], Epsilon));
                }
            }
        }
    }
    SECTION("Model Matrix Batch - AVX512")
    {
        if (!FPlatformMisc::GetCPUInformation().AVX512)
        {
            SKIP("AVX512 is not supported on this CPU");
        }

        int WorkedCount = Math::ComputeModelMatrixBatch_AVX512(
            Count, PositionX.Raw(), PositionY.Raw(), PositionZ.Raw(), QuaternionX.Raw(), QuaternionY.Raw(),
            QuaternionZ.Raw(), QuaternionW.Raw(), ScaleX.Raw(), ScaleY.Raw(), ScaleZ.Raw(),
            reinterpret_cast<TestType*>(OutModelMatrix.Raw()));

        if (Count == 8 && std::is_same_v<float, TestType>)
        {
            REQUIRE(WorkedCount == 0);    // AVX512 float should refuse 8 elements
        }
        else
        {
            REQUIRE(WorkedCount == Count);
        }

        INFO("Count: " << Count);
        for (size_t Index = 0; Index < WorkedCount; Index++)
        {
            INFO("Model matrix: " << Transform.GetModelMatrix());
            INFO("SIMD Model matrix: [" << Index << "]: " << OutModelMatrix[Index]);
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    INFO("Result[" << Index << "[" << i << "][" << j << "]: " << OutModelMatrix[Index][i][j]);
                    INFO("ExpectedResult[" << i << "][" << j << "]: " << Transform.GetModelMatrix()[i][j]);
                    CHECK_THAT(OutModelMatrix[Index][i][j],
                               Catch::Matchers::WithinRel(Transform.GetModelMatrix()[i][j], Epsilon));
                }
            }
        }
    }
}

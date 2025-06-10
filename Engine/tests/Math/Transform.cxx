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

template <typename T>
void PrepareModelMatrixBatch(const size_t Count, const TTransform<T> Source, TArray<T, 64>& PositionX,
                             TArray<T, 64>& PositionY, TArray<T, 64>& PositionZ, TArray<T, 64>& QuaternionX,
                             TArray<T, 64>& QuaternionY, TArray<T, 64>& QuaternionZ, TArray<T, 64>& QuaternionW,
                             TArray<T, 64>& ScaleX, TArray<T, 64>& ScaleY, TArray<T, 64>& ScaleZ)
{
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
        PositionX[i] = Source.GetLocation().x;
        PositionY[i] = Source.GetLocation().y;
        PositionZ[i] = Source.GetLocation().z;
        QuaternionX[i] = Source.GetRotation().x;
        QuaternionY[i] = Source.GetRotation().y;
        QuaternionZ[i] = Source.GetRotation().z;
        QuaternionW[i] = Source.GetRotation().w;
        ScaleX[i] = Source.GetScale().x;
        ScaleY[i] = Source.GetScale().y;
        ScaleZ[i] = Source.GetScale().z;
    }
}

TEST_CASE("Transform Matrices - float", "[Math][Transform][Float]")
{
    const float Epsilon = 1e-5f;
    const glm::vec3 Location{GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                             GENERATE(take(1, random(-50.0f, 50.0f)))};
    const glm::quat Rotation{GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                             GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f)))};
    const glm::vec3 Scale{GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                          GENERATE(take(1, random(-50.0f, 50.0f)))};

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
                CHECK_THAT(TranslationMatrix[i][j],
                           Catch::Matchers::WithinRel(ExpectedTranslationMatrix[i][j], Epsilon));
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
                CHECK_THAT(RotationMatrix[i][j], Catch::Matchers::WithinRel(ExpectedRotationMatrix[i][j], Epsilon));
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
                CHECK_THAT(ScaleMatrix[i][j], Catch::Matchers::WithinRel(ExpectedScaleMatrix[i][j], Epsilon));
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
                CHECK_THAT(ModelMatrix[i][j], Catch::Matchers::WithinRel(ExpectedModelMatrix[i][j], Epsilon));
            }
        }
    }

    SECTION("Model Matrix Batch - AVX2")
    {
        if (!FPlatformMisc::GetCPUInformation().AVX2)
        {
            SKIP("AVX2 is not supported on this CPU");
        }

        const size_t Count = GENERATE(8, 16, 32);
        TArray<float, 64> PositionX, PositionY, PositionZ;
        TArray<float, 64> QuaternionX, QuaternionY, QuaternionZ, QuaternionW;
        TArray<float, 64> ScaleX, ScaleY, ScaleZ;
        TArray<FMatrix4, 64> OutModelMatrix(Count);

        PrepareModelMatrixBatch(Count, Transform, PositionX, PositionY, PositionZ, QuaternionX, QuaternionY,
                                QuaternionZ, QuaternionW, ScaleX, ScaleY, ScaleZ);

        int WorkedCount = Math::ComputeModelMatrixBatch_AVX2(
            Count, PositionX.Raw(), PositionY.Raw(), PositionZ.Raw(), QuaternionX.Raw(), QuaternionY.Raw(),
            QuaternionZ.Raw(), QuaternionW.Raw(), ScaleX.Raw(), ScaleY.Raw(), ScaleZ.Raw(),
            reinterpret_cast<float*>(OutModelMatrix.Raw()));
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

        const size_t Count = GENERATE(8, 16, 32);
        TArray<float, 64> PositionX, PositionY, PositionZ;
        TArray<float, 64> QuaternionX, QuaternionY, QuaternionZ, QuaternionW;
        TArray<float, 64> ScaleX, ScaleY, ScaleZ;
        TArray<FMatrix4, 64> OutModelMatrix(Count);

        PrepareModelMatrixBatch(Count, Transform, PositionX, PositionY, PositionZ, QuaternionX, QuaternionY,
                                QuaternionZ, QuaternionW, ScaleX, ScaleY, ScaleZ);

        int WorkedCount = Math::ComputeModelMatrixBatch_AVX512(
            Count, PositionX.Raw(), PositionY.Raw(), PositionZ.Raw(), QuaternionX.Raw(), QuaternionY.Raw(),
            QuaternionZ.Raw(), QuaternionW.Raw(), ScaleX.Raw(), ScaleY.Raw(), ScaleZ.Raw(),
            reinterpret_cast<float*>(OutModelMatrix.Raw()));
        if (Count == 8)
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

TEST_CASE("Transform Matrices - double", "[Math][Transform][Double]")
{
    const double Epsilon = 1e-12f;
    const glm::dvec3 Location{GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                              GENERATE(take(1, random(-50.0f, 50.0f)))};
    const glm::dquat Rotation{GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                              GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f)))};
    const glm::dvec3 Scale{GENERATE(take(1, random(-50.0f, 50.0f))), GENERATE(take(1, random(-50.0f, 50.0f))),
                           GENERATE(take(1, random(-50.0f, 50.0f)))};

    DTransform Transform(DVector3{Location.x, Location.y, Location.z},
                         DQuaternion(Rotation.w, Rotation.x, Rotation.y, Rotation.z),
                         DVector3{Scale.x, Scale.y, Scale.z});

    SECTION("Translation Matrix")
    {
        const glm::dmat4 ExpectedTranslationMatrix = glm::translate(glm::dmat4(1.0f), Location);
        const DMatrix4 TranslationMatrix = Transform.GetTranslationMatrix();

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
        const glm::dmat4 ExpectedRotationMatrix = glm::toMat4(Rotation);
        const DMatrix4 RotationMatrix = Transform.GetRotationMatrix();

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
        const glm::dmat4 ExpectedScaleMatrix = glm::scale(glm::dmat4(1.0f), Scale);
        const DMatrix4 ScaleMatrix = Transform.GetScaleMatrix();

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
        const glm::dmat4 ExpectedTransformMatrix = glm::translate(glm::dmat4(1.0f), Location);
        const glm::dmat4 ExpectedRotationMatrix = glm::toMat4(Rotation);
        const glm::dmat4 ExpectedScaleMatrix = glm::scale(glm::dmat4(1.0f), Scale);

        const DMatrix4 TransformMatrix = Transform.GetTranslationMatrix();
        const DMatrix4 RotationMatrix = Transform.GetRotationMatrix();
        const DMatrix4 ScaleMatrix = Transform.GetScaleMatrix();

        const glm::dmat4 ExpectedModelMatrix = ExpectedTransformMatrix * ExpectedRotationMatrix * ExpectedScaleMatrix;
        const DMatrix4 ModelMatrix = Transform.GetModelMatrix();

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

    SECTION("Model Matrix Batch - AVX2")
    {
        if (!FPlatformMisc::GetCPUInformation().AVX2)
        {
            SKIP("AVX2 is not supported on this CPU");
        }

        const size_t Count = GENERATE(8, 16, 32);
        TArray<double, 64> PositionX, PositionY, PositionZ;
        TArray<double, 64> QuaternionX, QuaternionY, QuaternionZ, QuaternionW;
        TArray<double, 64> ScaleX, ScaleY, ScaleZ;
        TArray<DMatrix4, 64> OutModelMatrix(Count);

        PrepareModelMatrixBatch(Count, Transform, PositionX, PositionY, PositionZ, QuaternionX, QuaternionY,
                                QuaternionZ, QuaternionW, ScaleX, ScaleY, ScaleZ);

        int WorkedCount = Math::ComputeModelMatrixBatch_AVX2(
            Count, PositionX.Raw(), PositionY.Raw(), PositionZ.Raw(), QuaternionX.Raw(), QuaternionY.Raw(),
            QuaternionZ.Raw(), QuaternionW.Raw(), ScaleX.Raw(), ScaleY.Raw(), ScaleZ.Raw(),
            reinterpret_cast<double*>(OutModelMatrix.Raw()));
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

        const size_t Count = GENERATE(8, 16, 32);
        TArray<double, 64> PositionX, PositionY, PositionZ;
        TArray<double, 64> QuaternionX, QuaternionY, QuaternionZ, QuaternionW;
        TArray<double, 64> ScaleX, ScaleY, ScaleZ;
        TArray<DMatrix4, 64> OutModelMatrix(Count);

        PrepareModelMatrixBatch(Count, Transform, PositionX, PositionY, PositionZ, QuaternionX, QuaternionY,
                                QuaternionZ, QuaternionW, ScaleX, ScaleY, ScaleZ);

        int WorkedCount = Math::ComputeModelMatrixBatch_AVX512(
            Count, PositionX.Raw(), PositionY.Raw(), PositionZ.Raw(), QuaternionX.Raw(), QuaternionY.Raw(),
            QuaternionZ.Raw(), QuaternionW.Raw(), ScaleX.Raw(), ScaleY.Raw(), ScaleZ.Raw(),
            reinterpret_cast<double*>(OutModelMatrix.Raw()));
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
}

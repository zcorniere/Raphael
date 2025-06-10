#include "Engine/Math/Transform.hxx"

#include <immintrin.h>

namespace Math
{

[[gnu::target("avx2")]]
size_t ComputeModelMatrixBatch_AVX2(size_t Count, const float* RESTRICT PositionX, const float* RESTRICT PositionY,
                                    const float* RESTRICT PositionZ, const float* RESTRICT QuaternionX,
                                    const float* RESTRICT QuaternionY, const float* RESTRICT QuaternionZ,
                                    const float* RESTRICT QuaternionW, const float* RESTRICT ScaleX,
                                    const float* RESTRICT ScaleY, const float* RESTRICT ScaleZ,
                                    float* RESTRICT OutModelMatrix)
{
    RPH_PROFILE_FUNC()
    size_t i = 0;
    for (; i + 7 < Count; i += 8)
    {
        const __m256 X = _mm256_load_ps(QuaternionX + i);
        const __m256 Y = _mm256_load_ps(QuaternionY + i);
        const __m256 Z = _mm256_load_ps(QuaternionZ + i);
        const __m256 W = _mm256_load_ps(QuaternionW + i);
        const __m256 SX = _mm256_load_ps(ScaleX + i);
        const __m256 SY = _mm256_load_ps(ScaleY + i);
        const __m256 SZ = _mm256_load_ps(ScaleZ + i);
        const __m256 one = _mm256_set1_ps(1.0f);
        const __m256 two = _mm256_set1_ps(2.0f);

        const __m256 xx = _mm256_mul_ps(X, X);
        const __m256 yy = _mm256_mul_ps(Y, Y);
        const __m256 zz = _mm256_mul_ps(Z, Z);
        const __m256 xy = _mm256_mul_ps(X, Y);
        const __m256 xz = _mm256_mul_ps(X, Z);
        const __m256 yz = _mm256_mul_ps(Y, Z);
        const __m256 wx = _mm256_mul_ps(W, X);
        const __m256 wy = _mm256_mul_ps(W, Y);
        const __m256 wz = _mm256_mul_ps(W, Z);

        // MATCH SCALAR VERSION!
        __m256 m00 = _mm256_sub_ps(one, _mm256_mul_ps(two, _mm256_add_ps(yy, zz)));
        __m256 m01 = _mm256_mul_ps(two, _mm256_add_ps(xy, wz));
        __m256 m02 = _mm256_mul_ps(two, _mm256_sub_ps(xz, wy));

        __m256 m10 = _mm256_mul_ps(two, _mm256_sub_ps(xy, wz));
        __m256 m11 = _mm256_sub_ps(one, _mm256_mul_ps(two, _mm256_add_ps(xx, zz)));
        __m256 m12 = _mm256_mul_ps(two, _mm256_add_ps(yz, wx));

        __m256 m20 = _mm256_mul_ps(two, _mm256_add_ps(xz, wy));
        __m256 m21 = _mm256_mul_ps(two, _mm256_sub_ps(yz, wx));
        __m256 m22 = _mm256_sub_ps(one, _mm256_mul_ps(two, _mm256_add_ps(xx, yy)));

        // Apply scale
        m00 = _mm256_mul_ps(m00, SX);
        m01 = _mm256_mul_ps(m01, SX);
        m02 = _mm256_mul_ps(m02, SX);
        m10 = _mm256_mul_ps(m10, SY);
        m11 = _mm256_mul_ps(m11, SY);
        m12 = _mm256_mul_ps(m12, SY);
        m20 = _mm256_mul_ps(m20, SZ);
        m21 = _mm256_mul_ps(m21, SZ);
        m22 = _mm256_mul_ps(m22, SZ);

        // Store each matrix (row-major, 4x4)
        for (int j = 0; j < 8; ++j)
        {
            float* M = OutModelMatrix + (i + j) * 16;
            M[0] = ((float*)&m00)[j];
            M[1] = ((float*)&m01)[j];
            M[2] = ((float*)&m02)[j];
            M[3] = 0.0f;

            M[4] = ((float*)&m10)[j];
            M[5] = ((float*)&m11)[j];
            M[6] = ((float*)&m12)[j];
            M[7] = 0.0f;

            M[8] = ((float*)&m20)[j];
            M[9] = ((float*)&m21)[j];
            M[10] = ((float*)&m22)[j];
            M[11] = 0.0f;

            M[12] = PositionX[i + j];
            M[13] = PositionY[i + j];
            M[14] = PositionZ[i + j];
            M[15] = 1.0f;
        }
    }
    return i;
}

[[gnu::target("avx512f")]]
size_t ComputeModelMatrixBatch_AVX512(size_t Count, const float* RESTRICT PositionX, const float* RESTRICT PositionY,
                                      const float* RESTRICT PositionZ, const float* RESTRICT QuaternionX,
                                      const float* RESTRICT QuaternionY, const float* RESTRICT QuaternionZ,
                                      const float* RESTRICT QuaternionW, const float* RESTRICT ScaleX,
                                      const float* RESTRICT ScaleY, const float* RESTRICT ScaleZ,
                                      float* RESTRICT OutModelMatrix)
{
    RPH_PROFILE_FUNC()
    size_t i = 0;
    for (; i + 15 < Count; i += 16)
    {
        const __m512 X = _mm512_load_ps(QuaternionX + i);
        const __m512 Y = _mm512_load_ps(QuaternionY + i);
        const __m512 Z = _mm512_load_ps(QuaternionZ + i);
        const __m512 W = _mm512_load_ps(QuaternionW + i);
        const __m512 SX = _mm512_load_ps(ScaleX + i);
        const __m512 SY = _mm512_load_ps(ScaleY + i);
        const __m512 SZ = _mm512_load_ps(ScaleZ + i);
        const __m512 one = _mm512_set1_ps(1.0f);
        const __m512 two = _mm512_set1_ps(2.0f);

        const __m512 xx = _mm512_mul_ps(X, X);
        const __m512 yy = _mm512_mul_ps(Y, Y);
        const __m512 zz = _mm512_mul_ps(Z, Z);
        const __m512 xy = _mm512_mul_ps(X, Y);
        const __m512 xz = _mm512_mul_ps(X, Z);
        const __m512 yz = _mm512_mul_ps(Y, Z);
        const __m512 wx = _mm512_mul_ps(W, X);
        const __m512 wy = _mm512_mul_ps(W, Y);
        const __m512 wz = _mm512_mul_ps(W, Z);

        // MATCH SCALAR VERSION!
        __m512 m00 = _mm512_sub_ps(one, _mm512_mul_ps(two, _mm512_add_ps(yy, zz)));
        __m512 m01 = _mm512_mul_ps(two, _mm512_add_ps(xy, wz));
        __m512 m02 = _mm512_mul_ps(two, _mm512_sub_ps(xz, wy));

        __m512 m10 = _mm512_mul_ps(two, _mm512_sub_ps(xy, wz));
        __m512 m11 = _mm512_sub_ps(one, _mm512_mul_ps(two, _mm512_add_ps(xx, zz)));
        __m512 m12 = _mm512_mul_ps(two, _mm512_add_ps(yz, wx));

        __m512 m20 = _mm512_mul_ps(two, _mm512_add_ps(xz, wy));
        __m512 m21 = _mm512_mul_ps(two, _mm512_sub_ps(yz, wx));
        __m512 m22 = _mm512_sub_ps(one, _mm512_mul_ps(two, _mm512_add_ps(xx, yy)));

        // Apply scale
        m00 = _mm512_mul_ps(m00, SX);
        m01 = _mm512_mul_ps(m01, SX);
        m02 = _mm512_mul_ps(m02, SX);
        m10 = _mm512_mul_ps(m10, SY);
        m11 = _mm512_mul_ps(m11, SY);
        m12 = _mm512_mul_ps(m12, SY);
        m20 = _mm512_mul_ps(m20, SZ);
        m21 = _mm512_mul_ps(m21, SZ);
        m22 = _mm512_mul_ps(m22, SZ);

        // Store each matrix (row-major, 4x4)
        for (int j = 0; j < 16; ++j)
        {
            float* M = OutModelMatrix + (i + j) * 16;
            M[0] = ((float*)&m00)[j];
            M[1] = ((float*)&m01)[j];
            M[2] = ((float*)&m02)[j];
            M[3] = 0.0f;

            M[4] = ((float*)&m10)[j];
            M[5] = ((float*)&m11)[j];
            M[6] = ((float*)&m12)[j];
            M[7] = 0.0f;

            M[8] = ((float*)&m20)[j];
            M[9] = ((float*)&m21)[j];
            M[10] = ((float*)&m22)[j];
            M[11] = 0.0f;

            M[12] = PositionX[i + j];
            M[13] = PositionY[i + j];
            M[14] = PositionZ[i + j];
            M[15] = 1.0f;
        }
    }
    return i;
}

template <>
void ComputeModelMatrixBatch(const size_t Count, const float* PositionX, const float* PositionY, const float* PositionZ,
                             const float* QuaternionX, const float* QuaternionY, const float* QuaternionZ,
                             const float* QuaternionW, const float* ScaleX, const float* ScaleY, const float* ScaleZ,
                             TMatrix4<float>* OutModelMatrix)
{
    RPH_PROFILE_FUNC()
    check(Count > 0);
    ensure(PositionX && PositionY && PositionZ && QuaternionX && QuaternionY && QuaternionZ && QuaternionW && ScaleX &&
           ScaleY && ScaleZ && OutModelMatrix);

    size_t WorkedCount = 0;
    const FCPUInformation& CPUInfo = FPlatformMisc::GetCPUInformation();
    if (CPUInfo.AVX512 && Count >= 16)
    {
        const size_t BatchCount = (Count / 16) * 16;    // largest multiple of 16 <= Count
        if (BatchCount > 0)
        {
            WorkedCount += ComputeModelMatrixBatch_AVX512(
                BatchCount, PositionX + WorkedCount, PositionY + WorkedCount, PositionZ + WorkedCount,
                QuaternionX + WorkedCount, QuaternionY + WorkedCount, QuaternionZ + WorkedCount,
                QuaternionW + WorkedCount, ScaleX + WorkedCount, ScaleY + WorkedCount, ScaleZ + WorkedCount,
                reinterpret_cast<float*>(OutModelMatrix + WorkedCount));
        }
    }
    if (CPUInfo.AVX2 && Count >= 8)
    {
        const size_t Remaining = Count - WorkedCount;
        const size_t BatchCount = (Remaining / 8) * 8;
        if (BatchCount > 0)
        {
            WorkedCount += ComputeModelMatrixBatch_AVX2(
                Remaining, PositionX + WorkedCount, PositionY + WorkedCount, PositionZ + WorkedCount,
                QuaternionX + WorkedCount, QuaternionY + WorkedCount, QuaternionZ + WorkedCount,
                QuaternionW + WorkedCount, ScaleX + WorkedCount, ScaleY + WorkedCount, ScaleZ + WorkedCount,
                reinterpret_cast<float*>(OutModelMatrix + WorkedCount));
        }
    }

    // Fallback to scalar implementation
    for (; WorkedCount < Count; WorkedCount++)
    {
        const TVector3<float> Location(PositionX[WorkedCount], PositionY[WorkedCount], PositionZ[WorkedCount]);
        const TQuaternion<float> Rotation(QuaternionW[WorkedCount], QuaternionX[WorkedCount], QuaternionY[WorkedCount],
                                          QuaternionZ[WorkedCount]);
        const TVector3<float> Scale(ScaleX[WorkedCount], ScaleY[WorkedCount], ScaleZ[WorkedCount]);
        TTransform<float> Transform(Location, Rotation, Scale);

        OutModelMatrix[WorkedCount] = Transform.GetModelMatrix();
    }
}

}    // namespace Math

#include "Engine/Math/Transform.hxx"

#include <immintrin.h>

namespace Math
{

[[gnu::target("avx2")]]
size_t ComputeModelMatrixBatch_AVX2(size_t Count, const double* RESTRICT PositionX, const double* RESTRICT PositionY,
                                    const double* RESTRICT PositionZ, const double* RESTRICT QuaternionX,
                                    const double* RESTRICT QuaternionY, const double* RESTRICT QuaternionZ,
                                    const double* RESTRICT QuaternionW, const double* RESTRICT ScaleX,
                                    const double* RESTRICT ScaleY, const double* RESTRICT ScaleZ,
                                    double* RESTRICT OutModelMatrix)
{
    RPH_PROFILE_FUNC()
    size_t i = 0;
    for (; i + 3 < Count; i += 4)
    {
        const __m256 X = _mm256_load_pd(QuaternionX + i);
        const __m256 Y = _mm256_load_pd(QuaternionY + i);
        const __m256 Z = _mm256_load_pd(QuaternionZ + i);
        const __m256 W = _mm256_load_pd(QuaternionW + i);
        const __m256 SX = _mm256_load_pd(ScaleX + i);
        const __m256 SY = _mm256_load_pd(ScaleY + i);
        const __m256 SZ = _mm256_load_pd(ScaleZ + i);
        const __m256 one = _mm256_set1_pd(1.0f);
        const __m256 two = _mm256_set1_pd(2.0f);

        const __m256 xx = _mm256_mul_pd(X, X);
        const __m256 yy = _mm256_mul_pd(Y, Y);
        const __m256 zz = _mm256_mul_pd(Z, Z);
        const __m256 xy = _mm256_mul_pd(X, Y);
        const __m256 xz = _mm256_mul_pd(X, Z);
        const __m256 yz = _mm256_mul_pd(Y, Z);
        const __m256 wx = _mm256_mul_pd(W, X);
        const __m256 wy = _mm256_mul_pd(W, Y);
        const __m256 wz = _mm256_mul_pd(W, Z);

        // MATCH SCALAR VERSION!
        __m256 m00 = _mm256_sub_pd(one, _mm256_mul_pd(two, _mm256_add_pd(yy, zz)));
        __m256 m01 = _mm256_mul_pd(two, _mm256_add_pd(xy, wz));
        __m256 m02 = _mm256_mul_pd(two, _mm256_sub_pd(xz, wy));

        __m256 m10 = _mm256_mul_pd(two, _mm256_sub_pd(xy, wz));
        __m256 m11 = _mm256_sub_pd(one, _mm256_mul_pd(two, _mm256_add_pd(xx, zz)));
        __m256 m12 = _mm256_mul_pd(two, _mm256_add_pd(yz, wx));

        __m256 m20 = _mm256_mul_pd(two, _mm256_add_pd(xz, wy));
        __m256 m21 = _mm256_mul_pd(two, _mm256_sub_pd(yz, wx));
        __m256 m22 = _mm256_sub_pd(one, _mm256_mul_pd(two, _mm256_add_pd(xx, yy)));

        // Apply scale
        m00 = _mm256_mul_pd(m00, SX);
        m01 = _mm256_mul_pd(m01, SY);
        m02 = _mm256_mul_pd(m02, SZ);
        m10 = _mm256_mul_pd(m10, SX);
        m11 = _mm256_mul_pd(m11, SY);
        m12 = _mm256_mul_pd(m12, SZ);
        m20 = _mm256_mul_pd(m20, SX);
        m21 = _mm256_mul_pd(m21, SY);
        m22 = _mm256_mul_pd(m22, SZ);

        // Store each matrix (row-major, 4x4)
        for (int j = 0; j < 4; ++j)
        {
            double* M = OutModelMatrix + (i + j) * 16;
            M[0] = ((double*)&m00)[j];
            M[1] = ((double*)&m01)[j];
            M[2] = ((double*)&m02)[j];
            M[3] = 0.0f;

            M[4] = ((double*)&m10)[j];
            M[5] = ((double*)&m11)[j];
            M[6] = ((double*)&m12)[j];
            M[7] = 0.0f;

            M[8] = ((double*)&m20)[j];
            M[9] = ((double*)&m21)[j];
            M[10] = ((double*)&m22)[j];
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
size_t ComputeModelMatrixBatch_AVX512(size_t Count, const double* RESTRICT PositionX, const double* RESTRICT PositionY,
                                      const double* RESTRICT PositionZ, const double* RESTRICT QuaternionX,
                                      const double* RESTRICT QuaternionY, const double* RESTRICT QuaternionZ,
                                      const double* RESTRICT QuaternionW, const double* RESTRICT ScaleX,
                                      const double* RESTRICT ScaleY, const double* RESTRICT ScaleZ,
                                      double* RESTRICT OutModelMatrix)
{
    RPH_PROFILE_FUNC()
    size_t i = 0;
    for (; i + 7 < Count; i += 8)
    {
        const __m512 X = _mm512_load_pd(QuaternionX + i);
        const __m512 Y = _mm512_load_pd(QuaternionY + i);
        const __m512 Z = _mm512_load_pd(QuaternionZ + i);
        const __m512 W = _mm512_load_pd(QuaternionW + i);
        const __m512 SX = _mm512_load_pd(ScaleX + i);
        const __m512 SY = _mm512_load_pd(ScaleY + i);
        const __m512 SZ = _mm512_load_pd(ScaleZ + i);
        const __m512 one = _mm512_set1_pd(1.0f);
        const __m512 two = _mm512_set1_pd(2.0f);

        const __m512 xx = _mm512_mul_pd(X, X);
        const __m512 yy = _mm512_mul_pd(Y, Y);
        const __m512 zz = _mm512_mul_pd(Z, Z);
        const __m512 xy = _mm512_mul_pd(X, Y);
        const __m512 xz = _mm512_mul_pd(X, Z);
        const __m512 yz = _mm512_mul_pd(Y, Z);
        const __m512 wx = _mm512_mul_pd(W, X);
        const __m512 wy = _mm512_mul_pd(W, Y);
        const __m512 wz = _mm512_mul_pd(W, Z);

        // MATCH SCALAR VERSION!
        __m512 m00 = _mm512_sub_pd(one, _mm512_mul_pd(two, _mm512_add_pd(yy, zz)));
        __m512 m01 = _mm512_mul_pd(two, _mm512_add_pd(xy, wz));
        __m512 m02 = _mm512_mul_pd(two, _mm512_sub_pd(xz, wy));

        __m512 m10 = _mm512_mul_pd(two, _mm512_sub_pd(xy, wz));
        __m512 m11 = _mm512_sub_pd(one, _mm512_mul_pd(two, _mm512_add_pd(xx, zz)));
        __m512 m12 = _mm512_mul_pd(two, _mm512_add_pd(yz, wx));

        __m512 m20 = _mm512_mul_pd(two, _mm512_add_pd(xz, wy));
        __m512 m21 = _mm512_mul_pd(two, _mm512_sub_pd(yz, wx));
        __m512 m22 = _mm512_sub_pd(one, _mm512_mul_pd(two, _mm512_add_pd(xx, yy)));

        // Apply scale
        m00 = _mm512_mul_pd(m00, SX);
        m01 = _mm512_mul_pd(m01, SY);
        m02 = _mm512_mul_pd(m02, SZ);
        m10 = _mm512_mul_pd(m10, SX);
        m11 = _mm512_mul_pd(m11, SY);
        m12 = _mm512_mul_pd(m12, SZ);
        m20 = _mm512_mul_pd(m20, SX);
        m21 = _mm512_mul_pd(m21, SY);
        m22 = _mm512_mul_pd(m22, SZ);

        // Store each matrix (row-major, 4x4)
        for (int j = 0; j < 8; ++j)
        {
            double* M = OutModelMatrix + (i + j) * 16;
            M[0] = ((double*)&m00)[j];
            M[1] = ((double*)&m01)[j];
            M[2] = ((double*)&m02)[j];
            M[3] = 0.0f;

            M[4] = ((double*)&m10)[j];
            M[5] = ((double*)&m11)[j];
            M[6] = ((double*)&m12)[j];
            M[7] = 0.0f;

            M[8] = ((double*)&m20)[j];
            M[9] = ((double*)&m21)[j];
            M[10] = ((double*)&m22)[j];
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
void ComputeModelMatrixBatch(const size_t Count, const double* PositionX, const double* PositionY,
                             const double* PositionZ, const double* QuaternionX, const double* QuaternionY,
                             const double* QuaternionZ, const double* QuaternionW, const double* ScaleX,
                             const double* ScaleY, const double* ScaleZ, TMatrix4<double>* OutModelMatrix)
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
                reinterpret_cast<double*>(OutModelMatrix + WorkedCount));
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
                reinterpret_cast<double*>(OutModelMatrix + WorkedCount));
        }
    }

    // Fallback to scalar implementation
    for (; WorkedCount < Count; WorkedCount++)
    {
        const TVector3<double> Location(PositionX[WorkedCount], PositionY[WorkedCount], PositionZ[WorkedCount]);
        const TQuaternion<double> Rotation(QuaternionW[WorkedCount], QuaternionX[WorkedCount], QuaternionY[WorkedCount],
                                           QuaternionZ[WorkedCount]);
        const TVector3<double> Scale(ScaleX[WorkedCount], ScaleY[WorkedCount], ScaleZ[WorkedCount]);
        TTransform<double> Transform(Location, Rotation, Scale);

        OutModelMatrix[WorkedCount] = Transform.GetModelMatrix();
    }
}

}    // namespace Math

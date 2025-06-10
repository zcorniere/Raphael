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
        const __m256d X = _mm256_load_pd(QuaternionX + i);
        const __m256d Y = _mm256_load_pd(QuaternionY + i);
        const __m256d Z = _mm256_load_pd(QuaternionZ + i);
        const __m256d W = _mm256_load_pd(QuaternionW + i);
        const __m256d SX = _mm256_load_pd(ScaleX + i);
        const __m256d SY = _mm256_load_pd(ScaleY + i);
        const __m256d SZ = _mm256_load_pd(ScaleZ + i);
        const __m256d one = _mm256_set1_pd(1.0f);
        const __m256d two = _mm256_set1_pd(2.0f);

        const __m256d xx = _mm256_mul_pd(X, X);
        const __m256d yy = _mm256_mul_pd(Y, Y);
        const __m256d zz = _mm256_mul_pd(Z, Z);
        const __m256d xy = _mm256_mul_pd(X, Y);
        const __m256d xz = _mm256_mul_pd(X, Z);
        const __m256d yz = _mm256_mul_pd(Y, Z);
        const __m256d wx = _mm256_mul_pd(W, X);
        const __m256d wy = _mm256_mul_pd(W, Y);
        const __m256d wz = _mm256_mul_pd(W, Z);

        // MATCH SCALAR VERSION!
        __m256d m00 = _mm256_sub_pd(one, _mm256_mul_pd(two, _mm256_add_pd(yy, zz)));
        __m256d m01 = _mm256_mul_pd(two, _mm256_add_pd(xy, wz));
        __m256d m02 = _mm256_mul_pd(two, _mm256_sub_pd(xz, wy));

        __m256d m10 = _mm256_mul_pd(two, _mm256_sub_pd(xy, wz));
        __m256d m11 = _mm256_sub_pd(one, _mm256_mul_pd(two, _mm256_add_pd(xx, zz)));
        __m256d m12 = _mm256_mul_pd(two, _mm256_add_pd(yz, wx));

        __m256d m20 = _mm256_mul_pd(two, _mm256_add_pd(xz, wy));
        __m256d m21 = _mm256_mul_pd(two, _mm256_sub_pd(yz, wx));
        __m256d m22 = _mm256_sub_pd(one, _mm256_mul_pd(two, _mm256_add_pd(xx, yy)));

        // Apply scale
        m00 = _mm256_mul_pd(m00, SX);
        m01 = _mm256_mul_pd(m01, SX);
        m02 = _mm256_mul_pd(m02, SX);
        m10 = _mm256_mul_pd(m10, SY);
        m11 = _mm256_mul_pd(m11, SY);
        m12 = _mm256_mul_pd(m12, SY);
        m20 = _mm256_mul_pd(m20, SZ);
        m21 = _mm256_mul_pd(m21, SZ);
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
        const __m512d X = _mm512_load_pd(QuaternionX + i);
        const __m512d Y = _mm512_load_pd(QuaternionY + i);
        const __m512d Z = _mm512_load_pd(QuaternionZ + i);
        const __m512d W = _mm512_load_pd(QuaternionW + i);
        const __m512d SX = _mm512_load_pd(ScaleX + i);
        const __m512d SY = _mm512_load_pd(ScaleY + i);
        const __m512d SZ = _mm512_load_pd(ScaleZ + i);
        const __m512d one = _mm512_set1_pd(1.0f);
        const __m512d two = _mm512_set1_pd(2.0f);

        const __m512d xx = _mm512_mul_pd(X, X);
        const __m512d yy = _mm512_mul_pd(Y, Y);
        const __m512d zz = _mm512_mul_pd(Z, Z);
        const __m512d xy = _mm512_mul_pd(X, Y);
        const __m512d xz = _mm512_mul_pd(X, Z);
        const __m512d yz = _mm512_mul_pd(Y, Z);
        const __m512d wx = _mm512_mul_pd(W, X);
        const __m512d wy = _mm512_mul_pd(W, Y);
        const __m512d wz = _mm512_mul_pd(W, Z);

        // MATCH SCALAR VERSION!
        __m512d m00 = _mm512_sub_pd(one, _mm512_mul_pd(two, _mm512_add_pd(yy, zz)));
        __m512d m01 = _mm512_mul_pd(two, _mm512_add_pd(xy, wz));
        __m512d m02 = _mm512_mul_pd(two, _mm512_sub_pd(xz, wy));

        __m512d m10 = _mm512_mul_pd(two, _mm512_sub_pd(xy, wz));
        __m512d m11 = _mm512_sub_pd(one, _mm512_mul_pd(two, _mm512_add_pd(xx, zz)));
        __m512d m12 = _mm512_mul_pd(two, _mm512_add_pd(yz, wx));

        __m512d m20 = _mm512_mul_pd(two, _mm512_add_pd(xz, wy));
        __m512d m21 = _mm512_mul_pd(two, _mm512_sub_pd(yz, wx));
        __m512d m22 = _mm512_sub_pd(one, _mm512_mul_pd(two, _mm512_add_pd(xx, yy)));

        // Apply scale
        m00 = _mm512_mul_pd(m00, SX);
        m01 = _mm512_mul_pd(m01, SX);
        m02 = _mm512_mul_pd(m02, SX);
        m10 = _mm512_mul_pd(m10, SY);
        m11 = _mm512_mul_pd(m11, SY);
        m12 = _mm512_mul_pd(m12, SY);
        m20 = _mm512_mul_pd(m20, SZ);
        m21 = _mm512_mul_pd(m21, SZ);
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

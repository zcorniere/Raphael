#include "Engine/Math/Transform.hxx"

#include <immintrin.h>

namespace Math
{

[[gnu::target("avx2")]]
void ComputeModelMatrix_AVX2(TMatrix4<float>& ModelMatrix, const TVector3<float>& Location,
                             const TQuaternion<float>& Rotation, const TVector3<float>& Scale)
{

    // Unpack quaternion
    float x = Rotation.x, y = Rotation.y, z = Rotation.z, w = Rotation.w;

    // Compute rotation matrix (classic formula)
    float xx = x * x, yy = y * y, zz = z * z;
    float xy = x * y, xz = x * z, yz = y * z;
    float wx = w * x, wy = w * y, wz = w * z;

    float rot[9] = {1.0f - 2.0f * (yy + zz), 2.0f * (xy - wz),        2.0f * (xz + wy),
                    2.0f * (xy + wz),        1.0f - 2.0f * (xx + zz), 2.0f * (yz - wx),
                    2.0f * (xz - wy),        2.0f * (yz + wx),        1.0f - 2.0f * (xx + yy)};

    // Prepare scale vector for three matrix columns
    __m256 scale_vec = _mm256_set_ps(Scale[2], Scale[1], Scale[0],    // for m[8], m[7], m[6]
                                     Scale[2], Scale[1], Scale[0],    // for m[5], m[4], m[3]
                                     Scale[2], Scale[1]               // for m[2], m[1]
    );
    // Prepare rotation (pad last element for 8-float load)
    float rot_pad[8] = {rot[0], rot[1], rot[2], rot[3], rot[4], rot[5], rot[6], rot[7]};
    __m256 rot_vec = _mm256_loadu_ps(rot_pad);

    // Multiply rotation by scale (element-wise)
    __m256 rot_scaled = _mm256_mul_ps(rot_vec, scale_vec);

    // Store scaled matrix back (first 8 elements)
    float rot_scaled_arr[9];
    _mm256_storeu_ps(rot_scaled_arr, rot_scaled);
    rot_scaled_arr[8] = rot[8] * Scale[2];

    // Fill the 4x4 row-major model matrix
    // Fill the 4x4 row-major model matrix
    ModelMatrix[0, 0] = rot_scaled_arr[0];
    ModelMatrix[0, 1] = rot_scaled_arr[1];
    ModelMatrix[0, 2] = rot_scaled_arr[2];
    ModelMatrix[0, 3] = 0.0f;
    ModelMatrix[1, 0] = rot_scaled_arr[3];
    ModelMatrix[1, 1] = rot_scaled_arr[4];
    ModelMatrix[1, 2] = rot_scaled_arr[5];
    ModelMatrix[1, 3] = 0.0f;
    ModelMatrix[2, 0] = rot_scaled_arr[6];
    ModelMatrix[2, 1] = rot_scaled_arr[7];
    ModelMatrix[2, 2] = rot_scaled_arr[8];
    ModelMatrix[2, 3] = 0.0f;
    ModelMatrix[3, 0] = Location[0];
    ModelMatrix[3, 1] = Location[1];
    ModelMatrix[3, 2] = Location[2];
    ModelMatrix[3, 3] = 1.0f;
}

template <>
TMatrix4<float> TTransform<float>::GetModelMatrix()
{
    if (!bModelMatrixDirty)
    {
        return ModelMatrix;
    }
    if (FPlatformMisc::GetCPUInformation().AVX2)
    {
        RPH_PROFILE_FUNC("TTransform<float>::GetModelMatrix - AVX")
        ComputeModelMatrix_AVX2(ModelMatrix, Location, Rotation, Scale);
    }
    else
    {
        RPH_PROFILE_FUNC()
        ModelMatrix = GetTranslationMatrix() * Rotation.GetRotationMatrix() * GetScaleMatrix();
    }

    Math::CheckNaN(ModelMatrix);
    bModelMatrixDirty = false;
    return ModelMatrix;
}

template <typename T>
size_t ComputeModelMatrixBatch_AVX2(size_t Count, const T* PositionX, const T* PositionY, const T* PositionZ,
                                    const T* QuaternionX, const T* QuaternionY, const T* QuaternionZ,
                                    const T* QuaternionW, const T* ScaleX, const T* ScaleY, const T* ScaleZ,
                                    T* OutModelMatrix);
template <typename T>
size_t ComputeModelMatrixBatch_AVX512(size_t Count, const T* PositionX, const T* PositionY, const T* PositionZ,
                                      const T* QuaternionX, const T* QuaternionY, const T* QuaternionZ,
                                      const T* QuaternionW, const T* ScaleX, const T* ScaleY, const T* ScaleZ,
                                      T* OutModelMatrix);

template <>
[[gnu::target("avx2")]]
size_t ComputeModelMatrixBatch_AVX2(size_t Count, const float* __restrict__ PositionX,
                                    const float* __restrict__ PositionY, const float* __restrict__ PositionZ,
                                    const float* __restrict__ QuaternionX, const float* __restrict__ QuaternionY,
                                    const float* __restrict__ QuaternionZ, const float* __restrict__ QuaternionW,
                                    const float* __restrict__ ScaleX, const float* __restrict__ ScaleY,
                                    const float* __restrict__ ScaleZ, float* __restrict__ OutModelMatrix)
{
    RPH_PROFILE_FUNC()
    size_t i = 0;
    for (i = 0; i + 7 < Count; i += 8)
    {
        __m256 X = _mm256_load_ps(QuaternionX + i);
        __m256 Y = _mm256_load_ps(QuaternionY + i);
        __m256 Z = _mm256_load_ps(QuaternionZ + i);
        __m256 W = _mm256_load_ps(QuaternionW + i);
        __m256 SX = _mm256_load_ps(ScaleX + i);
        __m256 SY = _mm256_load_ps(ScaleY + i);
        __m256 SZ = _mm256_load_ps(ScaleZ + i);
        __m256 TX = _mm256_load_ps(PositionX + i);
        __m256 TY = _mm256_load_ps(PositionY + i);
        __m256 TZ = _mm256_load_ps(PositionZ + i);
        __m256 one = _mm256_set1_ps(1.0f);
        __m256 two = _mm256_set1_ps(2.0f);

        __m256 xx = _mm256_mul_ps(X, X);
        __m256 yy = _mm256_mul_ps(Y, Y);
        __m256 zz = _mm256_mul_ps(Z, Z);
        __m256 xy = _mm256_mul_ps(X, Y);
        __m256 xz = _mm256_mul_ps(X, Z);
        __m256 yz = _mm256_mul_ps(Y, Z);
        __m256 wx = _mm256_mul_ps(W, X);
        __m256 wy = _mm256_mul_ps(W, Y);
        __m256 wz = _mm256_mul_ps(W, Z);

        __m256 m00 = _mm256_sub_ps(one, _mm256_mul_ps(two, _mm256_add_ps(yy, zz)));
        __m256 m01 = _mm256_mul_ps(two, _mm256_sub_ps(xy, wz));
        __m256 m02 = _mm256_mul_ps(two, _mm256_add_ps(xz, wy));
        __m256 m10 = _mm256_mul_ps(two, _mm256_add_ps(xy, wz));
        __m256 m11 = _mm256_sub_ps(one, _mm256_mul_ps(two, _mm256_add_ps(xx, zz)));
        __m256 m12 = _mm256_mul_ps(two, _mm256_sub_ps(yz, wx));
        __m256 m20 = _mm256_mul_ps(two, _mm256_sub_ps(xz, wy));
        __m256 m21 = _mm256_mul_ps(two, _mm256_add_ps(yz, wx));
        __m256 m22 = _mm256_sub_ps(one, _mm256_mul_ps(two, _mm256_add_ps(xx, yy)));

        // Apply scale
        m00 = _mm256_mul_ps(m00, SX);
        m01 = _mm256_mul_ps(m01, SY);
        m02 = _mm256_mul_ps(m02, SZ);
        m10 = _mm256_mul_ps(m10, SX);
        m11 = _mm256_mul_ps(m11, SY);
        m12 = _mm256_mul_ps(m12, SZ);
        m20 = _mm256_mul_ps(m20, SX);
        m21 = _mm256_mul_ps(m21, SY);
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
            M[12] = ((float*)&TX)[j];
            M[13] = ((float*)&TY)[j];
            M[14] = ((float*)&TZ)[j];
            M[15] = 1.0f;
        }
    }
    return i;
}

template <>
[[gnu::target("avx512f")]]
size_t ComputeModelMatrixBatch_AVX512(size_t Count, const float* __restrict__ PositionX,
                                      const float* __restrict__ PositionY, const float* __restrict__ PositionZ,
                                      const float* __restrict__ QuaternionX, const float* __restrict__ QuaternionY,
                                      const float* __restrict__ QuaternionZ, const float* __restrict__ QuaternionW,
                                      const float* __restrict__ ScaleX, const float* __restrict__ ScaleY,
                                      const float* __restrict__ ScaleZ, float* __restrict__ OutModelMatrix)
{
    RPH_PROFILE_FUNC()
    size_t i = 0;
    for (; i + 15 < Count; i += 16)
    {
        __m512 X = _mm512_load_ps(QuaternionX + i);
        __m512 Y = _mm512_load_ps(QuaternionY + i);
        __m512 Z = _mm512_load_ps(QuaternionZ + i);
        __m512 W = _mm512_load_ps(QuaternionW + i);
        __m512 SX = _mm512_load_ps(ScaleX + i);
        __m512 SY = _mm512_load_ps(ScaleY + i);
        __m512 SZ = _mm512_load_ps(ScaleZ + i);
        __m512 TX = _mm512_load_ps(PositionX + i);
        __m512 TY = _mm512_load_ps(PositionY + i);
        __m512 TZ = _mm512_load_ps(PositionZ + i);
        __m512 one = _mm512_set1_ps(1.0f);
        __m512 two = _mm512_set1_ps(2.0f);

        __m512 xx = _mm512_mul_ps(X, X);
        __m512 yy = _mm512_mul_ps(Y, Y);
        __m512 zz = _mm512_mul_ps(Z, Z);
        __m512 xy = _mm512_mul_ps(X, Y);
        __m512 xz = _mm512_mul_ps(X, Z);
        __m512 yz = _mm512_mul_ps(Y, Z);
        __m512 wx = _mm512_mul_ps(W, X);
        __m512 wy = _mm512_mul_ps(W, Y);
        __m512 wz = _mm512_mul_ps(W, Z);

        __m512 m00 = _mm512_sub_ps(one, _mm512_mul_ps(two, _mm512_add_ps(yy, zz)));
        __m512 m01 = _mm512_mul_ps(two, _mm512_sub_ps(xy, wz));
        __m512 m02 = _mm512_mul_ps(two, _mm512_add_ps(xz, wy));
        __m512 m10 = _mm512_mul_ps(two, _mm512_add_ps(xy, wz));
        __m512 m11 = _mm512_sub_ps(one, _mm512_mul_ps(two, _mm512_add_ps(xx, zz)));
        __m512 m12 = _mm512_mul_ps(two, _mm512_sub_ps(yz, wx));
        __m512 m20 = _mm512_mul_ps(two, _mm512_sub_ps(xz, wy));
        __m512 m21 = _mm512_mul_ps(two, _mm512_add_ps(yz, wx));
        __m512 m22 = _mm512_sub_ps(one, _mm512_mul_ps(two, _mm512_add_ps(xx, yy)));

        // Apply scale
        m00 = _mm512_mul_ps(m00, SX);
        m01 = _mm512_mul_ps(m01, SY);
        m02 = _mm512_mul_ps(m02, SZ);
        m10 = _mm512_mul_ps(m10, SX);
        m11 = _mm512_mul_ps(m11, SY);
        m12 = _mm512_mul_ps(m12, SZ);
        m20 = _mm512_mul_ps(m20, SX);
        m21 = _mm512_mul_ps(m21, SY);
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
            M[12] = ((float*)&TX)[j];
            M[13] = ((float*)&TY)[j];
            M[14] = ((float*)&TZ)[j];
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
        size_t BatchCount = (Count / 16) * 16;    // largest multiple of 16 <= Count
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
        size_t Remaining = Count - WorkedCount;
        size_t BatchCount = (Remaining / 8) * 8;
        if (BatchCount > 0)
        {
            WorkedCount += ComputeModelMatrixBatch_AVX2(
                Count, PositionX + WorkedCount, PositionY + WorkedCount, PositionZ + WorkedCount,
                QuaternionX + WorkedCount, QuaternionY + WorkedCount, QuaternionZ + WorkedCount,
                QuaternionW + WorkedCount, ScaleX + WorkedCount, ScaleY + WorkedCount, ScaleZ + WorkedCount,
                reinterpret_cast<float*>(OutModelMatrix + WorkedCount));
        }
    }

    // Fallback to scalar implementation
    for (; WorkedCount < Count; WorkedCount++)
    {
        TVector3<float> Location(PositionX[WorkedCount], PositionY[WorkedCount], PositionZ[WorkedCount]);
        TQuaternion<float> Rotation(QuaternionW[WorkedCount], QuaternionX[WorkedCount], QuaternionY[WorkedCount],
                                    QuaternionZ[WorkedCount]);
        TVector3<float> Scale(ScaleX[WorkedCount], ScaleY[WorkedCount], ScaleZ[WorkedCount]);
        TTransform<float> Transform(Location, Rotation, Scale);

        OutModelMatrix[WorkedCount] = Transform.GetModelMatrix();
    }
}

}    // namespace Math

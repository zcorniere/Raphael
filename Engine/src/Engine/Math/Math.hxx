#pragma once

#include <numbers>

namespace Math
{

// The math should be done, considering a right-handed coordinate system, with Z pointing up
constexpr static FVector3 UpVector = {0, 0, 1};
constexpr static FVector3 FrontVector = {1, 0, 0};
constexpr static FVector3 RightVector = {0, 1, 0};

template <typename T>
constexpr T DegreeToRadian(T Degree)
{
    return Degree * (T(std::numbers::pi_v<double>) / T(180));
}

template <typename T>
constexpr T RadianToDegree(T Radian)
{
    return Radian * (T(180) / T(std::numbers::pi_v<double>));
}

template <typename T, unsigned Size>
constexpr T Dot(const TVector<Size, T>& lhs, const TVector<Size, T>& rhs)
{
    const TVector<Size, T> ResultTemp(lhs * rhs);
    return ResultTemp.x + ResultTemp.y + ResultTemp.z;
}

template <typename T, unsigned Size>
constexpr TVector<Size, T> Normalize(const TVector<Size, T>& Vector)
{
    const T InverseSqrt = T(1) / Dot(Vector, Vector);
    return Vector * InverseSqrt;
}

template <typename T, unsigned Size>
constexpr TVector<Size, T> Cross(const TVector<Size, T>& x, const TVector<Size, T>& y)
{
    return TVector<Size, T>(x.y * y.z - y.y * x.z, x.z * y.x - y.z * x.x, x.x * y.y - y.x * x.y);
}

template <unsigned TRowsCol, typename T>
constexpr TMatrix<TRowsCol, TRowsCol, T> Inverse(const TMatrix<TRowsCol, TRowsCol, T>& Matrix)
{
    TMatrix<TRowsCol, TRowsCol, T> Result;
    for (unsigned i = 0; i < TRowsCol; i++) {
        for (unsigned j = 0; j < TRowsCol; j++) {
            Result[i, j] = Matrix[j, i];
        }
    }
    return Result;
}

}    // namespace Math

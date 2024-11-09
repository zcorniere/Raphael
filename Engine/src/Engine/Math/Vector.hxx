#pragma once

namespace Math
{

template <unsigned Size, typename T>
struct FVector;

template <unsigned Size, typename T>
constexpr std::strong_ordering IsVectorEqual(const FVector<Size, T>& lhs, const FVector<Size, T>& rhs);

template <typename T>
struct FVector<2, T> {
    using Type = T;
    static constexpr unsigned Length = 2;
    union {
        struct {
            T x, y;
        };
        T data[2];
    };

    FVector();
    FVector(T x, T y);
    FVector(T scalar);
    FVector(const FVector<2, T>& other);

    bool operator==(const FVector<2, T>& other) const;
    std::strong_ordering operator<=>(const FVector<2, T>& other) const;
};

template <typename T>
struct FVector<3, T> {
    using Type = T;
    static constexpr unsigned Length = 3;
    union {
        struct {
            T x, y, z;
        };
        struct {
            T r, g, b;
        };
        T data[3];
    };

    FVector();
    FVector(T x, T y, T z);
    FVector(T scalar);
    FVector(const FVector<3, T>& other);
    FVector(const FVector<2, T>& other, T z);

    bool operator==(const FVector<3, T>& other) const;
    std::strong_ordering operator<=>(const FVector<3, T>& other) const;
};

template <typename T>
struct FVector<4, T> {
    using Type = T;
    static constexpr unsigned Length = 4;
    union {
        struct {
            T x, y, z, w;
        };
        struct {
            T r, g, b, a;
        };
        T data[4];
    };

    FVector();
    FVector(T x, T y, T z, T w);
    FVector(T scalar);
    FVector(const FVector<4, T>& other);
    FVector(const FVector<3, T>& other, T w);
    FVector(const FVector<2, T>& other, T z, T w);
    FVector(const FVector<2, T>& other1, const FVector<2, T>& other2);

    bool operator==(const FVector<4, T>& other) const;
    std::strong_ordering operator<=>(const FVector<4, T>& other) const;
};

// FVector operations
template <unsigned Size, typename T>
FVector<Size, T> operator+(const FVector<Size, T>& lhs, const FVector<Size, T>& rhs);

template <unsigned Size, typename T>
FVector<Size, T> operator-(const FVector<Size, T>& lhs, const FVector<Size, T>& rhs);

template <unsigned Size, typename T>
FVector<Size, T> operator*(const FVector<Size, T>& lhs, const FVector<Size, T>& rhs);

template <unsigned Size, typename T>
FVector<Size, T> operator/(const FVector<Size, T>& lhs, const FVector<Size, T>& rhs);

template <unsigned Size, typename T>
FVector<Size, T> operator*(const FVector<Size, T>& lhs, T scalar);
template <unsigned Size, typename T>
FVector<Size, T> operator/(const FVector<Size, T>& lhs, T scalar);

}    // namespace Math

using FVector2 = Math::FVector<2, float>;
using FVector3 = Math::FVector<3, float>;
using FVector4 = Math::FVector<4, float>;

using DVector2 = Math::FVector<2, double>;
using DVector3 = Math::FVector<3, double>;
using DVector4 = Math::FVector<4, double>;

using IVector2 = Math::FVector<2, int32>;
using IVector3 = Math::FVector<3, int32>;
using IVector4 = Math::FVector<4, int32>;

using UVector2 = Math::FVector<2, uint32>;
using UVector3 = Math::FVector<3, uint32>;
using UVector4 = Math::FVector<4, uint32>;

#include "Vector.inl"

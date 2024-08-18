#pragma once

namespace Math
{

template <unsigned Size, typename T>
struct Vector;

template <unsigned Size, typename T>
constexpr std::strong_ordering IsVectorEqual(const Vector<Size, T>& lhs, const Vector<Size, T>& rhs);

template <typename T>
struct Vector<2, T> {
    using Type = T;
    static constexpr unsigned Length = 2;
    union {
        struct {
            T x, y;
        };
        T data[2];
    };

    Vector();
    Vector(T x, T y);
    Vector(T scalar);
    Vector(const Vector<2, T>& other);

    bool operator==(const Vector<2, T>& other) const;
    std::strong_ordering operator<=>(const Vector<2, T>& other) const;
};

template <typename T>
struct Vector<3, T> {
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

    Vector();
    Vector(T x, T y, T z);
    Vector(T scalar);
    Vector(const Vector<3, T>& other);
    Vector(const Vector<2, T>& other, T z);

    bool operator==(const Vector<3, T>& other) const;
    std::strong_ordering operator<=>(const Vector<3, T>& other) const;
};

template <typename T>
struct Vector<4, T> {
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

    Vector();
    Vector(T x, T y, T z, T w);
    Vector(T scalar);
    Vector(const Vector<4, T>& other);
    Vector(const Vector<3, T>& other, T w);
    Vector(const Vector<2, T>& other, T z, T w);
    Vector(const Vector<2, T>& other1, const Vector<2, T>& other2);

    bool operator==(const Vector<4, T>& other) const;
    std::strong_ordering operator<=>(const Vector<4, T>& other) const;
};

// Vector operations
template <unsigned Size, typename T>
Vector<Size, T> operator+(const Vector<Size, T>& lhs, const Vector<Size, T>& rhs);

template <unsigned Size, typename T>
Vector<Size, T> operator-(const Vector<Size, T>& lhs, const Vector<Size, T>& rhs);

template <unsigned Size, typename T>
Vector<Size, T> operator*(const Vector<Size, T>& lhs, const Vector<Size, T>& rhs);

template <unsigned Size, typename T>
Vector<Size, T> operator/(const Vector<Size, T>& lhs, const Vector<Size, T>& rhs);

template <unsigned Size, typename T>
Vector<Size, T> operator*(const Vector<Size, T>& lhs, T scalar);
template <unsigned Size, typename T>
Vector<Size, T> operator/(const Vector<Size, T>& lhs, T scalar);

}    // namespace Math

using FVector2 = Math::Vector<2, float>;
using FVector3 = Math::Vector<3, float>;
using FVector4 = Math::Vector<4, float>;

using DVector2 = Math::Vector<2, double>;
using DVector3 = Math::Vector<3, double>;
using DVector4 = Math::Vector<4, double>;

using IVector2 = Math::Vector<2, int32>;
using IVector3 = Math::Vector<3, int32>;
using IVector4 = Math::Vector<4, int32>;

using UVector2 = Math::Vector<2, uint32>;
using UVector3 = Math::Vector<3, uint32>;
using UVector4 = Math::Vector<4, uint32>;

#include "Vector.inl"

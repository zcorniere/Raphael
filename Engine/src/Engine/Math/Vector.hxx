#pragma once

namespace Math
{

template <unsigned Size, typename T>
struct TVector;

template <unsigned Size, typename T>
constexpr std::strong_ordering IsVectorEqual(const TVector<Size, T>& lhs, const TVector<Size, T>& rhs);

template <typename T>
struct TVector<2, T> {
    using Type = T;
    static constexpr unsigned Length = 2;
    union {
        struct {
            T x, y;
        };
        T data[2];
    };

    TVector();
    TVector(T x, T y);
    TVector(T scalar);
    TVector(const TVector<2, T>& other);

    T& operator[](unsigned index);
    const T& operator[](unsigned index) const;
};

template <typename T>
struct TVector<3, T> {
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

    TVector();
    TVector(T x, T y, T z);
    TVector(T scalar);
    TVector(const TVector<3, T>& other);
    TVector(const TVector<2, T>& other, T z);

    T& operator[](unsigned index);
    const T& operator[](unsigned index) const;
};

template <typename T>
struct TVector<4, T> {
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

    TVector();
    TVector(T x, T y, T z, T w);
    TVector(T scalar);
    TVector(const TVector<4, T>& other);
    TVector(const TVector<3, T>& other, T w);
    TVector(const TVector<2, T>& other, T z, T w);
    TVector(const TVector<2, T>& other1, const TVector<2, T>& other2);

    T& operator[](unsigned index);
    const T& operator[](unsigned index) const;
};

// TVector operations
template <unsigned Size, typename T>
constexpr TVector<Size, T> operator+(const TVector<Size, T>& lhs, const TVector<Size, T>& rhs);

template <unsigned Size, typename T>
constexpr TVector<Size, T> operator-(const TVector<Size, T>& lhs, const TVector<Size, T>& rhs);

template <unsigned Size, typename T>
constexpr TVector<Size, T> operator*(const TVector<Size, T>& lhs, const TVector<Size, T>& rhs);

template <unsigned Size, typename T>
constexpr TVector<Size, T> operator/(const TVector<Size, T>& lhs, const TVector<Size, T>& rhs);

template <unsigned Size, typename T>
constexpr TVector<Size, T> operator*(const TVector<Size, T>& lhs, T scalar);
template <unsigned Size, typename T>
constexpr TVector<Size, T> operator/(const TVector<Size, T>& lhs, T scalar);

template <unsigned Size, typename T>
constexpr std::strong_ordering operator<=>(const TVector<Size, T>& lhs, const TVector<Size, T>& rhs);
template <unsigned Size, typename T>
constexpr bool operator==(const TVector<Size, T>& lhs, const TVector<Size, T>& rhs);

}    // namespace Math

template <unsigned Size, typename T>
struct std::formatter<Math::TVector<Size, T>, char> : public std::formatter<T> {

    template <class FormatContext>
    auto format(const Math::TVector<Size, T>& Value, FormatContext& ctx) const
    {
        auto&& out = ctx.out();
        format_to(out, "[");
        for (unsigned i = 0; i < Value.Length; i++) {
            std::formatter<T>::format(Value.data[i], ctx);
            if (i != Value.Length - 1) {
                format_to(out, ", ");
            }
        }
        format_to(out, "]");
        return out;
    }
};

template <unsigned Size, typename T>
std::ostream& operator<<(std::ostream& os, const Math::TVector<Size, T>& m)
{
    os << std::format("{}", m);
    return os;
}

using FVector2 = Math::TVector<2, float>;
using FVector3 = Math::TVector<3, float>;
using FVector4 = Math::TVector<4, float>;

using DVector2 = Math::TVector<2, double>;
using DVector3 = Math::TVector<3, double>;
using DVector4 = Math::TVector<4, double>;

using IVector2 = Math::TVector<2, int32>;
using IVector3 = Math::TVector<3, int32>;
using IVector4 = Math::TVector<4, int32>;

using UVector2 = Math::TVector<2, uint32>;
using UVector3 = Math::TVector<3, uint32>;
using UVector4 = Math::TVector<4, uint32>;

#include "Vector.inl"

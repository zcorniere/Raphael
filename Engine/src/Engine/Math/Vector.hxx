#pragma once

#if RPH_NAN_CHECKS
    #include <cmath>
#endif    // RPH_NAN_CHECKS

DECLARE_RTTI_CLASSBUILDER_TEMPLATE(TVector2, T)
DECLARE_RTTI_CLASSBUILDER_TEMPLATE(TVector3, T)
DECLARE_RTTI_CLASSBUILDER_TEMPLATE(TVector4, T)

namespace Math
{

template <unsigned Size, typename T>
struct TVector;

template <unsigned Size, typename T>
constexpr std::strong_ordering IsVectorEqual(const TVector<Size, T>& lhs, const TVector<Size, T>& rhs);

template <typename T>
struct TVector<2, T> {
    DECLARE_TEMPLATE_RTTI(TVector2, T);

public:
    using Type = T;

    static constexpr unsigned Length = 2;
    union {
        struct {
            T x, y;
        };
        T data[2];
    };

    constexpr TVector();
    constexpr TVector(T x, T y);
    constexpr TVector(T scalar);
    constexpr TVector(const TVector<2, T>& other);

    T& operator[](unsigned index);
    T operator[](unsigned index) const;
};

template <typename T>
struct TVector<3, T> {
    DECLARE_TEMPLATE_RTTI(TVector3, T);

public:
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

    constexpr TVector();
    constexpr TVector(T x, T y, T z);
    constexpr TVector(T scalar);
    constexpr TVector(const TVector<3, T>& other);
    constexpr TVector(const TVector<2, T>& other, T z = 0);

    T& operator[](unsigned index);
    T operator[](unsigned index) const;
};

template <typename T>
struct TVector<4, T> {
    DECLARE_TEMPLATE_RTTI(TVector4, T);

public:
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

    constexpr TVector();
    constexpr TVector(T x, T y, T z, T w);
    constexpr TVector(T scalar);
    constexpr TVector(const TVector<4, T>& other);
    constexpr TVector(const TVector<3, T>& other, T w = 0);
    constexpr TVector(const TVector<2, T>& other, T z = 0, T w = 0);
    constexpr TVector(const TVector<2, T>& other1, const TVector<2, T>& other2);

    T& operator[](unsigned index);
    T operator[](unsigned index) const;
};

// TVector operations
template <unsigned Size, typename T>
constexpr TVector<Size, T> operator+(const TVector<Size, T>& lhs, const TVector<Size, T>& rhs);

template <unsigned Size, typename T>
constexpr TVector<Size, T> operator-(const TVector<Size, T>& lhs, const TVector<Size, T>& rhs);

template <unsigned Size, typename T>
constexpr TVector<Size, T> operator-(const TVector<Size, T>& lhs);

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

template <unsigned Size, typename T>
requires std::is_floating_point_v<T>
void CheckNaN(const TVector<Size, T>& v);

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

template <typename T>
using TVector2 = Math::TVector<2, T>;
template <typename T>
using TVector3 = Math::TVector<3, T>;
template <typename T>
using TVector4 = Math::TVector<4, T>;

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

RTTI_DECLARE_NAME_TEMPLATE(TVector2, float);
RTTI_DECLARE_NAME_TEMPLATE(TVector2, double);

RTTI_DECLARE_NAME_TEMPLATE(TVector3, float);
RTTI_DECLARE_NAME_TEMPLATE(TVector3, double);

RTTI_DECLARE_NAME_TEMPLATE(TVector4, float);
RTTI_DECLARE_NAME_TEMPLATE(TVector4, double);

RTTI_BEGIN_CLASS_DECLARATION_TEMPLATE(TVector2, T)
PROPERTY(x)
PROPERTY(y)
RTTI_END_CLASS_DECLARATION;

RTTI_BEGIN_CLASS_DECLARATION_TEMPLATE(TVector3, T)
PROPERTY(x)
PROPERTY(y)
PROPERTY(z)
RTTI_END_CLASS_DECLARATION;

RTTI_BEGIN_CLASS_DECLARATION_TEMPLATE(TVector4, T)
PROPERTY(x)
PROPERTY(y)
PROPERTY(z)
PROPERTY(w)
RTTI_END_CLASS_DECLARATION;

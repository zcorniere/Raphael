#pragma once

#include "Engine/Math/Matrix.hxx"

namespace Math
{

template <typename T>
requires std::is_floating_point_v<T>
struct TQuaternion {
    using Type = T;
    union {
        struct {
            T x, y, z, w;
        };
        T data[4];
    };

    constexpr TQuaternion();
    constexpr TQuaternion(T w, T x, T y, T z);

    constexpr TMatrix4<T> GetRotationMatrix() const;

    constexpr T Length() const;
    constexpr T Dot(const TQuaternion& Other) const;

    constexpr TQuaternion Inverse() const;
    constexpr void Inverse();

    constexpr TQuaternion Normalize() const;
    constexpr void Normalize();
};

template <typename T>
constexpr TQuaternion<T> operator*(const TQuaternion<T>& lhs, const TQuaternion<T>& rhs);

template <typename T>
constexpr bool operator==(const TQuaternion<T>& lhs, const TQuaternion<T>& rhs);

template <typename T>
void CheckNaN(const TQuaternion<T>& q);

}    // namespace Math

template <typename T>
struct std::formatter<Math::TQuaternion<T>, char>;

template <typename T>
std::ostream& operator<<(std::ostream& os, const Math::TQuaternion<T>& m);

template <typename T>
using TQuaternion = Math::TQuaternion<T>;

using FQuaternion = TQuaternion<float>;
using DQuaternion = TQuaternion<double>;

#include "Quaternion.inl"

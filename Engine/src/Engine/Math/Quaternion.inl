#include "Quaternion.hxx"

namespace Math
{

template <typename T>
constexpr TQuaternion<T>::TQuaternion()
{
    x = y = z = 0;
    w = 1;
    static_assert(sizeof(TQuaternion<T>) == 4 * sizeof(T), "TQuaternion<T> is wider than it should be");
}

template <typename T>
constexpr TQuaternion<T>::TQuaternion(T w, T x, T y, T z): x(x), y(y), z(z), w(w)
{
}

template <typename T>
constexpr TMatrix4<T> TQuaternion<T>::GetRotationMatrix() const
{
    TMatrix4<T> Result;

    T xx(x * x);
    T yy(y * y);
    T zz(z * z);
    T xz(x * z);
    T xy(x * y);
    T yz(y * z);
    T wx(w * x);
    T wy(w * y);
    T wz(w * z);

    Result[0, 0] = T(1) - T(2) * (yy + zz);
    Result[0, 1] = T(2) * (xy + wz);
    Result[0, 2] = T(2) * (xz - wy);
    Result[0, 3] = T(0);

    Result[1, 0] = T(2) * (xy - wz);
    Result[1, 1] = T(1) - T(2) * (xx + zz);
    Result[1, 2] = T(2) * (yz + wx);
    Result[1, 3] = T(0);

    Result[2, 0] = T(2) * (xz + wy);
    Result[2, 1] = T(2) * (yz - wx);
    Result[2, 2] = T(1) - T(2) * (xx + yy);
    Result[2, 3] = T(0);

    Result[3, 0] = T(0);
    Result[3, 1] = T(0);
    Result[3, 2] = T(0);
    Result[3, 3] = T(1);

    return Result;
}

template <typename T>
constexpr T TQuaternion<T>::Dot(const TQuaternion& Other) const
{
    return x * Other.x + y * Other.y + z * Other.z + w * Other.w;
}

template <typename T>
constexpr TQuaternion<T> TQuaternion<T>::Inverse() const
{
    TQuaternion<T> Result = *this;
    Result.Inverse();
    return Result;
}

template <typename T>
constexpr void TQuaternion<T>::Inverse()
{
    const T Norm = Dot(*this);
    assert(Norm != 0);

    x = -x / Norm;
    y = -y / Norm;
    z = -z / Norm;
    w = w / Norm;
}

template <typename T>
constexpr TQuaternion<T> TQuaternion<T>::Normalize() const
{
    TQuaternion<T> Result = *this;
    Result.Normalize();
    return Result;
}

template <typename T>
constexpr T TQuaternion<T>::Length() const
{
    return std::sqrt(Dot(*this));
}

template <typename T>
constexpr void TQuaternion<T>::Normalize()
{
    const T Length = this->Length();
    assert(Length != 0);

    const T Norm = T(1) / Length;

    x *= Norm;
    y *= Norm;
    z *= Norm;
    w *= Norm;
}

template <typename T>
constexpr bool operator==(const TQuaternion<T>& lhs, const TQuaternion<T>& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
}

template <typename T>
constexpr TQuaternion<T> operator*(const TQuaternion<T>& lhs, const TQuaternion<T>& rhs)
{
    TQuaternion<T> Result;

    Result.w = lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z;
    Result.x = lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y;
    Result.y = lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z;
    Result.z = lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x;
    return Result;
}

}    // namespace Math

template <typename T>
struct std::formatter<Math::TQuaternion<T>, char> : public std::formatter<T> {

    template <class FormatContext>
    auto format(const Math::TQuaternion<T>& Value, FormatContext& ctx) const
    {
        auto&& out = ctx.out();
        format_to(out, "Quaternion(");
        std::formatter<T>::format(Value.w, ctx);
        format_to(out, " {{");
        std::formatter<T>::format(Value.x, ctx);
        format_to(out, ", ");
        std::formatter<T>::format(Value.y, ctx);
        format_to(out, ", ");
        std::formatter<T>::format(Value.z, ctx);
        format_to(out, "}})");
        return out;
    }
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const Math::TQuaternion<T>& m)
{
    os << std::format("{:.6f}", m);
    return os;
}

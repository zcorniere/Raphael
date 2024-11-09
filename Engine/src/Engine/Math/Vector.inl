namespace Math
{

template <unsigned Size, typename T>
constexpr std::strong_ordering IsVectorEqual(const FVector<Size, T>& lhs, const FVector<Size, T>& rhs)
{
    for (unsigned i = 0; i < Size; ++i) {
        if (lhs.data[i] < rhs.data[i]) {
            return std::strong_ordering::less;
        } else if (lhs.data[i] > rhs.data[i]) {
            return std::strong_ordering::greater;
        }
    }
    return std::strong_ordering::equal;
}

// FVector 2
template <typename T>
FVector<2, T>::FVector()
{
    x = y = 0;
    static_assert(sizeof(FVector<2, T>) == 2 * sizeof(T));
}

template <typename T>
FVector<2, T>::FVector(T x, T y): x(x), y(y)
{
}

template <typename T>
FVector<2, T>::FVector(T scalar): x(scalar), y(scalar)
{
}

template <typename T>
FVector<2, T>::FVector(const FVector<2, T>& other): x(other.x), y(other.y)
{
}

template <typename T>
bool FVector<2, T>::operator==(const FVector<2, T>& other) const
{
    return IsVectorEqual(*this, other) == std::strong_ordering::equal;
}

template <typename T>
std::strong_ordering FVector<2, T>::operator<=>(const FVector<2, T>& other) const
{
    return IsVectorEqual(*this, other);
}
// End FVector 2

// FVector 3
template <typename T>
FVector<3, T>::FVector()
{
    x = y = z = 0;
    static_assert(sizeof(FVector<3, T>) == 3 * sizeof(T));
}

template <typename T>
FVector<3, T>::FVector(T x, T y, T z): x(x), y(y), z(z)
{
}

template <typename T>
FVector<3, T>::FVector(T scalar): x(scalar), y(scalar), z(scalar)
{
}

template <typename T>
FVector<3, T>::FVector(const FVector<3, T>& other): x(other.x), y(other.y), z(other.z)
{
}

template <typename T>
FVector<3, T>::FVector(const FVector<2, T>& other, T z): x(other.x), y(other.y), z(z)
{
}

template <typename T>
bool FVector<3, T>::operator==(const FVector<3, T>& other) const
{
    return IsVectorEqual(*this, other) == std::strong_ordering::equal;
}

template <typename T>
std::strong_ordering FVector<3, T>::operator<=>(const FVector<3, T>& other) const
{
    return IsVectorEqual(*this, other);
}
// End FVector 3

// FVector 4
template <typename T>
FVector<4, T>::FVector()
{
    x = y = z = w = 0;
    static_assert(sizeof(FVector<4, T>) == 4 * sizeof(T));
}

template <typename T>
FVector<4, T>::FVector(T x, T y, T z, T w): x(x), y(y), z(z), w(w)
{
}

template <typename T>
FVector<4, T>::FVector(T scalar): x(scalar), y(scalar), z(scalar), w(scalar)
{
}

template <typename T>
FVector<4, T>::FVector(const FVector<4, T>& other): x(other.x), y(other.y), z(other.z), w(other.w)
{
}

template <typename T>
FVector<4, T>::FVector(const FVector<3, T>& other, T w): x(other.x), y(other.y), z(other.z), w(w)
{
}

template <typename T>
FVector<4, T>::FVector(const FVector<2, T>& other, T z, T w): x(other.x), y(other.y), z(z), w(w)
{
}

template <typename T>
FVector<4, T>::FVector(const FVector<2, T>& other1, const FVector<2, T>& other2)
    : x(other1.x), y(other1.y), z(other2.x), w(other2.y)
{
}

template <typename T>
bool FVector<4, T>::operator==(const FVector<4, T>& other) const
{
    return IsVectorEqual(*this, other) == std::strong_ordering::equal;
}

template <typename T>
std::strong_ordering FVector<4, T>::operator<=>(const FVector<4, T>& other) const
{
    return IsVectorEqual(*this, other);
}
// End FVector 4

template <unsigned Size, typename T>
FVector<Size, T> operator+(const FVector<Size, T>& lhs, const FVector<Size, T>& rhs)
{
    FVector<Size, T> result;
    for (unsigned i = 0; i < Size; ++i) {
        result.data[i] = lhs.data[i] + rhs.data[i];
    }
    return result;
}

template <unsigned Size, typename T>
FVector<Size, T> operator-(const FVector<Size, T>& lhs, const FVector<Size, T>& rhs)
{
    FVector<Size, T> result;
    for (unsigned i = 0; i < Size; ++i) {
        result.data[i] = lhs.data[i] - rhs.data[i];
    }
    return result;
}

template <unsigned Size, typename T>
FVector<Size, T> operator*(const FVector<Size, T>& lhs, const FVector<Size, T>& rhs)
{
    FVector<Size, T> result;
    for (unsigned i = 0; i < Size; ++i) {
        result.data[i] = lhs.data[i] * rhs.data[i];
    }
    return result;
}

template <unsigned Size, typename T>
FVector<Size, T> operator/(const FVector<Size, T>& lhs, const FVector<Size, T>& rhs)
{
    FVector<Size, T> result;
    for (unsigned i = 0; i < Size; ++i) {
        result.data[i] = lhs.data[i] / rhs.data[i];
    }
    return result;
}

template <unsigned Size, typename T>
FVector<Size, T> operator*(const FVector<Size, T>& lhs, T scalar)
{
    FVector<Size, T> result;
    for (unsigned i = 0; i < Size; ++i) {
        result.data[i] = lhs.data[i] * scalar;
    }
    return result;
}

template <unsigned Size, typename T>
FVector<Size, T> operator/(const FVector<Size, T>& lhs, T scalar)
{
    FVector<Size, T> result;
    for (unsigned i = 0; i < Size; ++i) {
        result.data[i] = lhs.data[i] / scalar;
    }
    return result;
};

}    // namespace Math

namespace Math
{

// TVector 2
template <typename T>
TVector<2, T>::TVector()
{
    x = y = 0;
    static_assert(sizeof(TVector<2, T>) == 2 * sizeof(T), "TVector<2, T> is wider than it should be");
}

template <typename T>
TVector<2, T>::TVector(T x, T y): x(x), y(y)
{
}

template <typename T>
TVector<2, T>::TVector(T scalar): x(scalar), y(scalar)
{
}

template <typename T>
TVector<2, T>::TVector(const TVector<2, T>& other): x(other.x), y(other.y)
{
}

template <typename T>
T& TVector<2, T>::operator[](unsigned index)
{
    assert(index < 2);
    return data[index];
}

template <typename T>
const T& TVector<2, T>::operator[](unsigned index) const
{
    assert(index < 2);
    return data[index];
}
// End TVector 2

// TVector 3
template <typename T>
TVector<3, T>::TVector()
{
    x = y = z = 0;
    static_assert(sizeof(TVector<3, T>) == 3 * sizeof(T), "TVector<3, T> is wider than it should be");
}

template <typename T>
TVector<3, T>::TVector(T x, T y, T z): x(x), y(y), z(z)
{
}

template <typename T>
TVector<3, T>::TVector(T scalar): x(scalar), y(scalar), z(scalar)
{
}

template <typename T>
TVector<3, T>::TVector(const TVector<3, T>& other): x(other.x), y(other.y), z(other.z)
{
}

template <typename T>
TVector<3, T>::TVector(const TVector<2, T>& other, T z): x(other.x), y(other.y), z(z)
{
}

template <typename T>
T& TVector<3, T>::operator[](unsigned index)
{
    assert(index < 3);
    return data[index];
}

template <typename T>
const T& TVector<3, T>::operator[](unsigned index) const
{
    assert(index < 3);
    return data[index];
}
// End TVector 3

// TVector 4
template <typename T>
TVector<4, T>::TVector()
{
    x = y = z = w = 0;
    static_assert(sizeof(TVector<4, T>) == 4 * sizeof(T), "TVector<4, T> is wider than it should be");
}

template <typename T>
TVector<4, T>::TVector(T x, T y, T z, T w): x(x), y(y), z(z), w(w)
{
}

template <typename T>
TVector<4, T>::TVector(T scalar): x(scalar), y(scalar), z(scalar), w(scalar)
{
}

template <typename T>
TVector<4, T>::TVector(const TVector<4, T>& other): x(other.x), y(other.y), z(other.z), w(other.w)
{
}

template <typename T>
TVector<4, T>::TVector(const TVector<3, T>& other, T w): x(other.x), y(other.y), z(other.z), w(w)
{
}

template <typename T>
TVector<4, T>::TVector(const TVector<2, T>& other, T z, T w): x(other.x), y(other.y), z(z), w(w)
{
}

template <typename T>
TVector<4, T>::TVector(const TVector<2, T>& other1, const TVector<2, T>& other2)
    : x(other1.x), y(other1.y), z(other2.x), w(other2.y)
{
}

template <typename T>
T& TVector<4, T>::operator[](unsigned index)
{
    assert(index < 4);
    return data[index];
}

template <typename T>
const T& TVector<4, T>::operator[](unsigned index) const
{
    assert(index < 4);
    return data[index];
}
// End TVector 4

// Generic operators
template <unsigned Size, typename T>
constexpr TVector<Size, T> operator+(const TVector<Size, T>& lhs, const TVector<Size, T>& rhs)
{
    TVector<Size, T> result;
    for (unsigned i = 0; i < Size; i++) {
        result.data[i] = lhs.data[i] + rhs.data[i];
    }
    return result;
}

template <unsigned Size, typename T>
constexpr TVector<Size, T> operator-(const TVector<Size, T>& lhs, const TVector<Size, T>& rhs)
{
    TVector<Size, T> result;
    for (unsigned i = 0; i < Size; i++) {
        result.data[i] = lhs.data[i] - rhs.data[i];
    }
    return result;
}

template <unsigned Size, typename T>
constexpr TVector<Size, T> operator*(const TVector<Size, T>& lhs, const TVector<Size, T>& rhs)
{
    TVector<Size, T> result;
    for (unsigned i = 0; i < Size; i++) {
        result.data[i] = lhs.data[i] * rhs.data[i];
    }
    return result;
}

template <unsigned Size, typename T>
constexpr TVector<Size, T> operator/(const TVector<Size, T>& lhs, const TVector<Size, T>& rhs)
{
    TVector<Size, T> result;
    for (unsigned i = 0; i < Size; i++) {
        result.data[i] = lhs.data[i] / rhs.data[i];
    }
    return result;
}

template <unsigned Size, typename T>
constexpr TVector<Size, T> operator*(const TVector<Size, T>& lhs, T scalar)
{
    TVector<Size, T> result;
    for (unsigned i = 0; i < Size; i++) {
        result.data[i] = lhs.data[i] * scalar;
    }
    return result;
}

template <unsigned Size, typename T>
constexpr TVector<Size, T> operator/(const TVector<Size, T>& lhs, T scalar)
{
    TVector<Size, T> result;
    for (unsigned i = 0; i < Size; i++) {
        result.data[i] = lhs.data[i] / scalar;
    }
    return result;
};

template <unsigned Size, typename T>
constexpr std::strong_ordering operator<=>(const TVector<Size, T>& lhs, const TVector<Size, T>& rhs)
{
    for (unsigned i = 0; i < Size; i++) {
        if (lhs.data[i] < rhs.data[i]) {
            return std::strong_ordering::less;
        } else if (lhs.data[i] > rhs.data[i]) {
            return std::strong_ordering::greater;
        }
    }
    return std::strong_ordering::equal;
}

template <unsigned Size, typename T>
constexpr bool operator==(const TVector<Size, T>& lhs, const TVector<Size, T>& rhs)
{
    return (lhs <=> rhs) == std::strong_ordering::equal;
}

}    // namespace Math

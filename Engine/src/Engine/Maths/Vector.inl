namespace Math
{

template <unsigned Size, typename T>
constexpr std::strong_ordering IsVectorEqual(const Vector<Size, T>& lhs, const Vector<Size, T>& rhs)
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

// Vector 2
template <typename T>
Vector<2, T>::Vector()
{
    x = y = 0;
}

template <typename T>
Vector<2, T>::Vector(T x, T y): x(x), y(y)
{
}

template <typename T>
Vector<2, T>::Vector(T scalar): x(scalar), y(scalar)
{
}

template <typename T>
Vector<2, T>::Vector(const Vector<2, T>& other): x(other.x), y(other.y)
{
}

template <typename T>
bool Vector<2, T>::operator==(const Vector<2, T>& other) const
{
    return IsVectorEqual(*this, other) == std::strong_ordering::equal;
}

template <typename T>
std::strong_ordering Vector<2, T>::operator<=>(const Vector<2, T>& other) const
{
    return IsVectorEqual(*this, other);
}
// End Vector 2

// Vector 3
template <typename T>
Vector<3, T>::Vector()
{
    x = y = z = 0;
}

template <typename T>
Vector<3, T>::Vector(T x, T y, T z): x(x), y(y), z(z)
{
}

template <typename T>
Vector<3, T>::Vector(T scalar): x(scalar), y(scalar), z(scalar)
{
}

template <typename T>
Vector<3, T>::Vector(const Vector<3, T>& other): x(other.x), y(other.y), z(other.z)
{
}

template <typename T>
Vector<3, T>::Vector(const Vector<2, T>& other, T z): x(other.x), y(other.y), z(z)
{
}

template <typename T>
std::strong_ordering Vector<3, T>::operator<=>(const Vector<3, T>& other) const
{
    return IsVectorEqual(*this, other);
}
// End Vector 3

// Vector 4
template <typename T>
Vector<4, T>::Vector()
{
    x = y = z = w = 0;
}

template <typename T>
Vector<4, T>::Vector(T x, T y, T z, T w): x(x), y(y), z(z), w(w)
{
}

template <typename T>
Vector<4, T>::Vector(T scalar): x(scalar), y(scalar), z(scalar), w(scalar)
{
}

template <typename T>
Vector<4, T>::Vector(const Vector<4, T>& other): x(other.x), y(other.y), z(other.z), w(other.w)
{
}

template <typename T>
Vector<4, T>::Vector(const Vector<3, T>& other, T w): x(other.x), y(other.y), z(other.z), w(w)
{
}

template <typename T>
Vector<4, T>::Vector(const Vector<2, T>& other, T z, T w): x(other.x), y(other.y), z(z), w(w)
{
}

template <typename T>
Vector<4, T>::Vector(const Vector<2, T>& other1, const Vector<2, T>& other2)
    : x(other1.x), y(other1.y), z(other2.x), w(other2.y)
{
}

template <typename T>
std::strong_ordering Vector<4, T>::operator<=>(const Vector<4, T>& other) const
{
    return IsVectorEqual(*this, other);
}
// End Vector 4

template <unsigned Size, typename T>
Vector<Size, T> operator+(const Vector<Size, T>& lhs, const Vector<Size, T>& rhs)
{
    Vector<Size, T> result;
    for (unsigned i = 0; i < Size; ++i) {
        result.data[i] = lhs.data[i] + rhs.data[i];
    }
    return result;
}

template <unsigned Size, typename T>
Vector<Size, T> operator-(const Vector<Size, T>& lhs, const Vector<Size, T>& rhs)
{
    Vector<Size, T> result;
    for (unsigned i = 0; i < Size; ++i) {
        result.data[i] = lhs.data[i] - rhs.data[i];
    }
    return result;
}

template <unsigned Size, typename T>
Vector<Size, T> operator*(const Vector<Size, T>& lhs, const Vector<Size, T>& rhs)
{
    Vector<Size, T> result;
    for (unsigned i = 0; i < Size; ++i) {
        result.data[i] = lhs.data[i] * rhs.data[i];
    }
    return result;
}

template <unsigned Size, typename T>
Vector<Size, T> operator/(const Vector<Size, T>& lhs, const Vector<Size, T>& rhs)
{
    Vector<Size, T> result;
    for (unsigned i = 0; i < Size; ++i) {
        result.data[i] = lhs.data[i] / rhs.data[i];
    }
    return result;
}

template <unsigned Size, typename T>
Vector<Size, T> operator*(const Vector<Size, T>& lhs, T scalar)
{
    Vector<Size, T> result;
    for (unsigned i = 0; i < Size; ++i) {
        result.data[i] = lhs.data[i] * scalar;
    }
    return result;
}

}    // namespace Math

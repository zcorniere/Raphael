#pragma once

namespace Math
{

template <unsigned Size, typename T>
struct TVector;

template <unsigned TRows, unsigned TColumns, typename T>
struct TMatrix {
    using Type = T;
    static constexpr const unsigned Rows = TRows;
    static constexpr const unsigned Columns = TColumns;

    static constexpr TMatrix Identity();

public:
    constexpr TMatrix(const Type& InDefaultValue = 0);

    constexpr bool IsSquare() const
    {
        return Rows == Columns;
    }

    constexpr TVector<TColumns, T>& operator[](unsigned Index);
    constexpr const TVector<TColumns, T>& operator[](unsigned Index) const;

private:
    TVector<Columns, T> Data[Rows];
};

template <unsigned TRows, unsigned TColumns, typename T>
constexpr TMatrix<TRows, TColumns, T> operator+(const TMatrix<TRows, TColumns, T>& lhs,
                                                const TMatrix<TRows, TColumns, T>& rhs);

template <unsigned TRows, unsigned TColumns, typename T>
constexpr TMatrix<TRows, TColumns, T> operator-(const TMatrix<TRows, TColumns, T>& lhs,
                                                const TMatrix<TRows, TColumns, T>& rhs);

template <unsigned TRows, unsigned TColumns, typename T>
constexpr TMatrix<TRows, TColumns, T> operator*(const TMatrix<TRows, TColumns, T>& lhs,
                                                const TMatrix<TRows, TColumns, T>& rhs);

template <unsigned TRows, unsigned TColumns, typename T>
constexpr TMatrix<TRows, TColumns, T> operator/(const TMatrix<TRows, TColumns, T>& lhs,
                                                const TMatrix<TRows, TColumns, T>& rhs);

template <unsigned TRows, unsigned TColumns, typename T>
constexpr bool operator==(const TMatrix<TRows, TColumns, T>& lhs, const TMatrix<TRows, TColumns, T>& rhs);

}    // namespace Math

template <unsigned TRows, unsigned TColumns, typename T>
struct std::formatter<Math::TMatrix<TRows, TColumns, T>, char>;

template <unsigned TRows, unsigned TColumns, typename T>
std::ostream& operator<<(std::ostream& os, const Math::TMatrix<TRows, TColumns, T>& m);

using FMatrix2 = Math::TMatrix<2, 2, float>;
using FMatrix3 = Math::TMatrix<3, 3, float>;
using FMatrix4 = Math::TMatrix<4, 4, float>;

using DMatrix2 = Math::TMatrix<2, 2, double>;
using DMatrix3 = Math::TMatrix<3, 3, double>;
using DMatrix4 = Math::TMatrix<4, 4, double>;

#include "Matrix.inl"

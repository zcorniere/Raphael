namespace Math
{

template <unsigned TRows, unsigned TColumns, typename T>
requires(TRows > 0 && TColumns > 0)
constexpr TMatrix<TRows, TColumns, T> TMatrix<TRows, TColumns, T>::Identity(const T Value)
requires(TMatrix::IsSquare())
{
    TMatrix Result(T(0));
    for (int i = 0; i < TRows; ++i)
    {
        Result[i][i] = Value;
    }
    return Result;
}

template <unsigned TRows, unsigned TColumns, typename T>
requires(TRows > 0 && TColumns > 0)
constexpr TMatrix<TRows, TColumns, T>::TMatrix()
{
    std::memset(Data, 0, sizeof(Data));
    static_assert(sizeof(TMatrix) == Rows * Columns * sizeof(T), "TMatrix is wider than it should be");
}

template <unsigned TRows, unsigned TColumns, typename T>
requires(TRows > 0 && TColumns > 0)
constexpr TMatrix<TRows, TColumns, T>::TMatrix(const Type& InDefaultValue)
{
    static_assert(sizeof(TMatrix) == Rows * Columns * sizeof(T), "TMatrix is wider than it should be");

    for (unsigned i = 0; i < Rows; i++)
    {
        for (unsigned j = 0; j < Columns; j++)
        {
            Data[i][j] = InDefaultValue;
        }
    }
}

template <unsigned TRows, unsigned TColumns, typename T>
requires(TRows > 0 && TColumns > 0)
constexpr TVector<TColumns, T>& TMatrix<TRows, TColumns, T>::operator[](unsigned Index)
{
    assert(Index < TRows);
    return Data[Index];
}

template <unsigned TRows, unsigned TColumns, typename T>
requires(TRows > 0 && TColumns > 0)
constexpr const TVector<TColumns, T>& TMatrix<TRows, TColumns, T>::operator[](unsigned Index) const
{
    assert(Index < TRows);
    return Data[Index];
}

template <unsigned TRows, unsigned TColumns, typename T>
requires(TRows > 0 && TColumns > 0)
constexpr T& TMatrix<TRows, TColumns, T>::operator[](unsigned Row, unsigned Column)
{
    assert(Row < TRows && Column < TColumns);
    return Data[Row][Column];
}

template <unsigned TRows, unsigned TColumns, typename T>
requires(TRows > 0 && TColumns > 0)
constexpr T TMatrix<TRows, TColumns, T>::operator[](unsigned Row, unsigned Column) const
{
    assert(Row < TRows && Column < TColumns);
    return Data[Row][Column];
}

template <unsigned TRows, unsigned TColumns, typename T>
requires(TRows > 0 && TColumns > 0)
constexpr TMatrix<TRows, TColumns, T> operator+(const TMatrix<TRows, TColumns, T>& lhs,
                                                const TMatrix<TRows, TColumns, T>& rhs)
{
    TMatrix<TRows, TColumns, T> Result;
    for (size_t i = 0; i < TRows; ++i)
    {
        for (size_t j = 0; j < TColumns; ++j)
        {
            Result[i][j] = lhs[i][j] + rhs[i][j];
        }
    }
    return Result;
}

template <unsigned TRows, unsigned TColumns, typename T>
requires(TRows > 0 && TColumns > 0)
constexpr TMatrix<TRows, TColumns, T> operator-(const TMatrix<TRows, TColumns, T>& lhs,
                                                const TMatrix<TRows, TColumns, T>& rhs)
{
    TMatrix<TRows, TColumns, T> Result;
    for (size_t i = 0; i < TRows; ++i)
    {
        for (size_t j = 0; j < TColumns; ++j)
        {
            Result[i][j] = lhs[i][j] - rhs[i][j];
        }
    }
    return Result;
}

template <unsigned TRows, unsigned TColumns, typename T>
requires(TRows > 0 && TColumns > 0)
constexpr TMatrix<TRows, TColumns, T> operator-(const TMatrix<TRows, TColumns, T>& lhs)
{
    TMatrix<TRows, TColumns, T> Result;
    for (size_t i = 0; i < TRows; ++i)
    {
        for (size_t j = 0; j < TColumns; ++j)
        {
            Result[i][j] = -lhs[i][j];
        }
    }
    return Result;
}

template <unsigned TRows, unsigned TColumns, typename T>
requires(TRows > 0 && TColumns > 0)
constexpr TMatrix<TRows, TColumns, T> operator*(const TMatrix<TRows, TColumns, T>& lhs,
                                                const TMatrix<TRows, TColumns, T>& rhs)
{
    TMatrix<TRows, TColumns, T> Result;
    for (unsigned i = 0; i < TRows; i++)
    {
        for (unsigned j = 0; j < TColumns; j++)
        {
            for (unsigned k = 0; k < TRows; k++)
            {
                Result[i][k] += lhs[j][k] * rhs[i][j];
            }
        }
    }
    return Result;
}

template <unsigned TRows, unsigned TColumns, typename T>
requires(TRows > 0 && TColumns > 0)
constexpr TVector<TColumns, T> operator*(const TMatrix<TRows, TColumns, T>& lhs, const TVector<TColumns, T>& rhs)
{
    TVector<TColumns, T> Result;
    for (size_t i = 0; i < TRows; ++i)
    {
        T Sum = T{};
        for (size_t j = 0; j < TColumns; ++j)
        {
            Sum += lhs[i][j] * rhs[j];
        }
        Result[i] = Sum;
    }
    return Result;
}

template <unsigned TRows, unsigned TColumns, typename T>
requires(TRows > 0 && TColumns > 0)
constexpr bool operator==(const TMatrix<TRows, TColumns, T>& lhs, const TMatrix<TRows, TColumns, T>& rhs)
{
    for (unsigned i = 0; i < TRows; i++)
    {
        for (unsigned j = 0; j < TColumns; j++)
        {
            if (lhs[i][j] != rhs[i][j])
            {
                return false;
            }
        }
    }
    return true;
}

template <unsigned TRows, unsigned TColumns, typename T>
requires(TRows > 0 && TColumns > 0 && std::is_floating_point_v<T>)
void CheckNaN(const TMatrix<TRows, TColumns, T>& m)
{
#if RPH_NAN_CHECKS
    for (unsigned i = 0; i < TRows; i++)
    {
        for (unsigned j = 0; j < TColumns; j++)
        {
            ensureAlwaysMsg(!std::isnan(m[i, j]), "NaN detected in Matrix<{}, {}, {}> at index {}", TRows, TColumns,
                            RTTI::TypeName<T>(), i);
        }
    }
#else
    (void)m;
#endif    // RPH_NAN_CHECKS
}

}    // namespace Math

template <unsigned TRows, unsigned TColumns, typename T>
requires(TRows > 0 && TColumns > 0)
struct std::formatter<Math::TMatrix<TRows, TColumns, T>, char> : public std::formatter<Math::TVector<TColumns, T>>
{
    template <class FormatContext>
    auto format(const Math::TMatrix<TRows, TColumns, T>& Value, FormatContext& ctx) const
    {
        auto&& out = ctx.out();
        format_to(out, "Matrix<{}, {}>(", TRows, TColumns);
        for (unsigned i = 0; i < TRows; i++)
        {
            std::formatter<Math::TVector<TColumns, T>>::format(Value[i], ctx);
            if (i + 1 < TRows)
            {
                format_to(out, ", ");
            }
        }
        format_to(out, ")");
        return out;
    }
};

template <unsigned TRows, unsigned TColumns, typename T>
requires(TRows > 0 && TColumns > 0)
std::ostream& operator<<(std::ostream& os, const Math::TMatrix<TRows, TColumns, T>& m)
{
    os << std::format("{:.6f}", m);
    return os;
}

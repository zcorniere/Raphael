namespace Math
{

template <unsigned TRows, unsigned TColumns, typename T>
constexpr TMatrix<TRows, TColumns, T> TMatrix<TRows, TColumns, T>::Identity()
{
    static_assert(TRows == TColumns, "Identity matrix must be square");

    TMatrix Result;
    for (int i = 0; i < TRows; ++i) {
        Result[i][i] = 1.0f;
    }
    return Result;
}

template <unsigned TRows, unsigned TColumns, typename T>
constexpr TMatrix<TRows, TColumns, T>::TMatrix(const Type& InDefaultValue)
{
    static_assert(sizeof(TMatrix) == Rows * Columns * sizeof(T), "TMatrix is wider than it should be");

    if (InDefaultValue == T(0)) {
        std::memset(Data, 0, sizeof(Data));
    } else {
        for (unsigned i = 0; i < Rows; i++) {
            for (unsigned j = 0; j < Columns; j++) {
                Data[i][j] = InDefaultValue;
            }
        }
    }
}

template <unsigned TRows, unsigned TColumns, typename T>
constexpr TVector<TColumns, T>& TMatrix<TRows, TColumns, T>::operator[](unsigned Index)
{
    return Data[Index];
}

template <unsigned TRows, unsigned TColumns, typename T>
constexpr const TVector<TColumns, T>& TMatrix<TRows, TColumns, T>::operator[](unsigned Index) const
{
    return Data[Index];
}

template <unsigned TRows, unsigned TColumns, typename T>
constexpr TMatrix<TRows, TColumns, T> operator+(const TMatrix<TRows, TColumns, T>& lhs,
                                                const TMatrix<TRows, TColumns, T>& rhs)
{
    TMatrix<TRows, TColumns, T> Result;
    for (size_t i = 0; i < TRows; ++i) {
        for (size_t j = 0; j < TColumns; ++j) {
            Result[i][j] = lhs[i][j] + rhs[i][j];
        }
    }
    return Result;
}

template <unsigned TRows, unsigned TColumns, typename T>
constexpr TMatrix<TRows, TColumns, T> operator-(const TMatrix<TRows, TColumns, T>& lhs,
                                                const TMatrix<TRows, TColumns, T>& rhs)
{
    TMatrix<TRows, TColumns, T> Result;
    for (size_t i = 0; i < TRows; ++i) {
        for (size_t j = 0; j < TColumns; ++j) {
            Result[i][j] = lhs[i][j] - rhs[i][j];
        }
    }
    return Result;
}

template <unsigned TRows, unsigned TColumns, typename T>
constexpr TMatrix<TRows, TColumns, T> operator*(const TMatrix<TRows, TColumns, T>& lhs,
                                                const TMatrix<TRows, TColumns, T>& rhs)
{
    TMatrix<TRows, TColumns, T> Result;
    for (size_t i = 0; i < TRows; ++i) {
        for (size_t j = 0; j < TColumns; ++j) {
            T Sum = T{};
            for (size_t k = 0; k < TColumns; ++k) {
                Sum += lhs[i][k] * rhs[k][j];
            }
            Result[i][j] = Sum;
        }
    }
    return Result;
}

template <unsigned TRows, unsigned TColumns, typename T>
constexpr TMatrix<TRows, TColumns, T> operator/(const TMatrix<TRows, TColumns, T>& lhs,
                                                const TMatrix<TRows, TColumns, T>& rhs)
{
    TMatrix<TRows, TColumns, T> Result;
    for (size_t i = 0; i < TRows; ++i) {
        for (size_t j = 0; j < TColumns; ++j) {
            T Sum = T{};
            for (size_t k = 0; k < TColumns; ++k) {
                if (rhs[k][j] == T{}) {
                    Sum = std::numeric_limits<T>::Nan();
                    break;
                }
                Sum += lhs[i][k] / rhs[k][j];
            }
            Result[i][j] = Sum;
        }
    }
    return Result;
}

template <unsigned TRows, unsigned TColumns, typename T>
constexpr bool operator==(const TMatrix<TRows, TColumns, T>& lhs, const TMatrix<TRows, TColumns, T>& rhs)
{
    for (unsigned i = 0; i < TRows; i++) {
        for (unsigned j = 0; j < TColumns; j++) {
            if (lhs[i][j] != rhs[i][j]) {
                return false;
            }
        }
    }
    return true;
}

}    // namespace Math

template <unsigned TRows, unsigned TColumns, typename T>
struct std::formatter<Math::TMatrix<TRows, TColumns, T>, char> : public std::formatter<Math::TVector<TColumns, T>> {
    template <class FormatContext>
    auto format(const Math::TMatrix<TRows, TColumns, T>& Value, FormatContext& ctx) const
    {
        auto&& out = ctx.out();
        format_to(out, "Matrix<{}, {}>(", TRows, TColumns);
        for (unsigned i = 0; i < TRows; i++) {
            std::formatter<Math::TVector<TColumns, T>>::format(Value[i], ctx);
            if (i + 1 < TRows) {
                format_to(out, ", ");
            }
        }
        format_to(out, ")");
        return out;
    }
};

template <unsigned TRows, unsigned TColumns, typename T>
std::ostream& operator<<(std::ostream& os, const Math::TMatrix<TRows, TColumns, T>& m)
{
    os << std::format("{:.6f}", m);
    return os;
}

#pragma once

namespace Math
{

template <unsigned Size, typename T>
struct FVector;

template <unsigned TRows, unsigned TColumns, typename T>
struct TMatrix {
    using Type = T;
    static constexpr const unsigned Rows = TRows;
    static constexpr const unsigned Columns = TColumns;

public:
    TMatrix(const Type& InDefaultValue = 0)
    {
        std::memset(Data, InDefaultValue, sizeof(Data));
    }

    constexpr bool IsSquare() const
    {
        return Rows == Columns;
    }

private:
    Type Data[Rows][Columns];
};

}    // namespace Math

using FMatrix4 = Math::TMatrix<4, 4, float>;

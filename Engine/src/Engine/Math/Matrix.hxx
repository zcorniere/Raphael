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
    static constexpr const bool IsSquare = Rows == Columns;

private:
    FVector<Rows, Type> data[Columns] = {};
};

}    // namespace Math

using FMatrix4 = Math::TMatrix<4, 4, float>;

#pragma once

namespace Math
{

template <unsigned Size, typename T>
struct Vector;

template <unsigned TRows, unsigned TColumns, typename T>
struct Matrix {
    using Type = T;
    static constexpr const unsigned Rows = TRows;
    static constexpr const unsigned Columns = TColumns;
    static constexpr const bool IsSquare = Rows == Columns;

private:
    Vector<Rows, Type> data[Columns] = {};
};

}    // namespace Math

using FMatrix4 = Math::Matrix<4, 4, float>;

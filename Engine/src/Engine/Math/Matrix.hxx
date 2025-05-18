#pragma once

#if RPH_NAN_CHECKS
    #include <cmath>
#endif    // RPH_NAN_CHECKS

DECLARE_RTTI_CLASSBUILDER_TEMPLATE(TMatrix2, T)
DECLARE_RTTI_CLASSBUILDER_TEMPLATE(TMatrix3, T)
DECLARE_RTTI_CLASSBUILDER_TEMPLATE(TMatrix4, T)

namespace Math
{

template <unsigned Size, typename T>
struct TVector;

template <unsigned TRows, unsigned TColumns, typename T>
struct TMatrix {
    DECLARE_TEMPLATE_RTTI_NO_FRIEND(TMatrix, T, public);

public:
    using Type = T;
    using ColumnType = TVector<TColumns, T>;

    static constexpr const unsigned Rows = TRows;
    static constexpr const unsigned Columns = TColumns;

    static constexpr TMatrix Identity();

    static const RTTI::FName& GetTypeName()
    {
        static const RTTI::FName NameHolder(InitName());
        return NameHolder;
    }
    static std::string InitName()
    {
        return std::format("Matrix<{}, {}, {}>", TRows, TColumns, RTTI::TypeName<T>());
    }

public:
    constexpr TMatrix();
    constexpr TMatrix(const Type& InDefaultValue);

    template <typename... ArgsType>
    requires((std::is_same_v<T, ArgsType> && ...) && sizeof...(ArgsType) == TRows * TColumns)
    constexpr TMatrix(ArgsType... Args)
    {
        unsigned i = 0;
        unsigned j = 0;
        auto AdderLambda = [this, &i, &j](T Type) {
            Data[i][j++] = Type;
            if (j / TColumns) {
                j = 0;
                i++;
            }
        };

        (AdderLambda(Args), ...);
    }

    static constexpr bool IsSquare()
    {
        return Rows == Columns;
    }

    constexpr TVector<TColumns, T>& operator[](unsigned Index);
    constexpr const TVector<TColumns, T>& operator[](unsigned Index) const;

    constexpr T& operator[](unsigned Row, unsigned Column);
    constexpr T operator[](unsigned Row, unsigned Column) const;

private:
    ColumnType Data[Rows];
};

template <unsigned TRows, unsigned TColumns, typename T>
constexpr TMatrix<TRows, TColumns, T> operator+(const TMatrix<TRows, TColumns, T>& lhs,
                                                const TMatrix<TRows, TColumns, T>& rhs);

template <unsigned TRows, unsigned TColumns, typename T>
constexpr TMatrix<TRows, TColumns, T> operator-(const TMatrix<TRows, TColumns, T>& lhs,
                                                const TMatrix<TRows, TColumns, T>& rhs);

template <unsigned TRows, unsigned TColumns, typename T>
constexpr TMatrix<TRows, TColumns, T> operator-(const TMatrix<TRows, TColumns, T>& lhs);

template <unsigned TRows, unsigned TColumns, typename T>
constexpr TMatrix<TRows, TColumns, T> operator*(const TMatrix<TRows, TColumns, T>& lhs,
                                                const TMatrix<TRows, TColumns, T>& rhs);

template <unsigned TRows, unsigned TColumns, typename T>
constexpr TVector<TColumns, T> operator*(const TMatrix<TRows, TColumns, T>& lhs, const TVector<TColumns, T>& rhs);

template <unsigned TRows, unsigned TColumns, typename T>
constexpr TMatrix<TRows, TColumns, T> operator/(const TMatrix<TRows, TColumns, T>& lhs,
                                                const TMatrix<TRows, TColumns, T>& rhs);

template <unsigned TRows, unsigned TColumns, typename T>
constexpr bool operator==(const TMatrix<TRows, TColumns, T>& lhs, const TMatrix<TRows, TColumns, T>& rhs);

template <unsigned TRows, unsigned TColumns, typename T>
requires std::is_floating_point_v<T>
void CheckNaN(const TMatrix<TRows, TColumns, T>& m);

}    // namespace Math

template <unsigned TRows, unsigned TColumns, typename T>
struct std::formatter<Math::TMatrix<TRows, TColumns, T>, char>;

template <unsigned TRows, unsigned TColumns, typename T>
std::ostream& operator<<(std::ostream& os, const Math::TMatrix<TRows, TColumns, T>& m);

template <typename T>
using TMatrix2 = Math::TMatrix<2, 2, T>;
template <typename T>
using TMatrix3 = Math::TMatrix<3, 3, T>;
template <typename T>
using TMatrix4 = Math::TMatrix<4, 4, T>;

using FMatrix2 = TMatrix2<float>;
using FMatrix3 = TMatrix3<float>;
using FMatrix4 = TMatrix4<float>;

using DMatrix2 = TMatrix2<double>;
using DMatrix3 = TMatrix3<double>;
using DMatrix4 = TMatrix4<double>;

RTTI_DECLARE_NAME_TEMPLATE(TMatrix2, float);
RTTI_DECLARE_NAME_TEMPLATE(TMatrix2, double);

RTTI_DECLARE_NAME_TEMPLATE(TMatrix3, float);
RTTI_DECLARE_NAME_TEMPLATE(TMatrix3, double);

RTTI_DECLARE_NAME_TEMPLATE(TMatrix4, float);
RTTI_DECLARE_NAME_TEMPLATE(TMatrix4, double);

RTTI_BEGIN_CLASS_DECLARATION_TEMPLATE(TMatrix2, T)
RTTI_END_CLASS_DECLARATION
RTTI_BEGIN_CLASS_DECLARATION_TEMPLATE(TMatrix3, T)
RTTI_END_CLASS_DECLARATION
RTTI_BEGIN_CLASS_DECLARATION_TEMPLATE(TMatrix4, T)
RTTI_END_CLASS_DECLARATION

#include "Matrix.inl"

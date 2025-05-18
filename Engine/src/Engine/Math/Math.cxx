#include "Engine/Math/Transform.hxx"

template <typename T>
RTTI::FClass* Math::TVector<2, T>::StaticClass = nullptr;
IMPLEMENT_RTTI_TEMPLATE(TVector2, float)
IMPLEMENT_RTTI_TEMPLATE(TVector2, double)

template <typename T>
RTTI::FClass* Math::TVector<3, T>::StaticClass = nullptr;
IMPLEMENT_RTTI_TEMPLATE(TVector3, float)
IMPLEMENT_RTTI_TEMPLATE(TVector3, double)

template <typename T>
RTTI::FClass* Math::TVector<4, T>::StaticClass = nullptr;
IMPLEMENT_RTTI_TEMPLATE(TVector4, float)
IMPLEMENT_RTTI_TEMPLATE(TVector4, double)

template <typename T>
requires std::is_floating_point_v<T>
RTTI::FClass* Math::TQuaternion<T>::StaticClass = nullptr;
IMPLEMENT_RTTI_TEMPLATE(TQuaternion, float)
IMPLEMENT_RTTI_TEMPLATE(TQuaternion, double)

template <unsigned TRows, unsigned TColumns, typename T>
RTTI::FClass* Math::TMatrix<TRows, TColumns, T>::StaticClass = nullptr;
IMPLEMENT_RTTI_TEMPLATE(TMatrix2, float);
IMPLEMENT_RTTI_TEMPLATE(TMatrix2, double)
IMPLEMENT_RTTI_TEMPLATE(TMatrix3, float);
IMPLEMENT_RTTI_TEMPLATE(TMatrix3, double)
IMPLEMENT_RTTI_TEMPLATE(TMatrix4, float);
IMPLEMENT_RTTI_TEMPLATE(TMatrix4, double)

IMPLEMENT_RTTI_TEMPLATE_STATIC_CLASS(Math::TTransform, T)
IMPLEMENT_RTTI_TEMPLATE(TTransform, float)
IMPLEMENT_RTTI_TEMPLATE(TTransform, double)

namespace Math
{

void RegisterMathTypes()
{
    // Would love to make it constexpr, but it's not possible with the current C++ standard
    // accessing a union member is not allowed in a constant expression context
    LOG(LogMath, Trace, "Making sure the math still make sense: {} == {}", Math::RightVector,
        Math::Cross(Math::UpVector, Math::FrontVector));
    check(Math::RightVector == Math::Cross(Math::UpVector, Math::FrontVector));
}

}    // namespace Math

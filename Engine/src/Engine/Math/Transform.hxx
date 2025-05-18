#pragma once

#include "Engine/Math/Matrix.hxx"
#include "Engine/Math/Quaternion.hxx"
#include "Engine/Math/Vector.hxx"

DECLARE_RTTI_CLASSBUILDER_TEMPLATE(TTransform, T)

namespace Math
{

template <typename T>
class TTransform
{
    DECLARE_TEMPLATE_RTTI(TTransform, T)
    RTTI_DECLARE_TYPEINFO_MINIMAL(TTransform)
public:
    TTransform() = default;
    TTransform(const TVector3<T>& InLocation, const TQuaternion<T>& InRotation, const TVector3<T>& InScale);

    void SetLocation(const TVector3<T>& InLocation);
    void SetRotation(const TQuaternion<T>& InRotation);
    void SetScale(const TVector3<T>& InScale);

    const TVector3<T>& GetLocation() const;
    const TQuaternion<T>& GetRotation() const;
    const TVector3<T>& GetScale() const;

    TVector3<T>& GetLocation();
    TQuaternion<T>& GetRotation();
    TVector3<T>& GetScale();

    TMatrix4<T> GetModelMatrix();

    TMatrix4<T> GetTranslationMatrix() const;
    TMatrix4<T> GetRotationMatrix() const;
    TMatrix4<T> GetScaleMatrix() const;

public:
    TVector3<T> Location = {0, 0, 0};
    TQuaternion<T> Rotation = {0, 0, 0, 1};
    TVector3<T> Scale = {1, 1, 1};

    bool bModelMatrixDirty = true;
    TMatrix4<T> ModelMatrix;
};

}    // namespace Math

template <typename T>
using TTransform = Math::TTransform<T>;

RTTI_DECLARE_NAME_TEMPLATE(TTransform, float);
RTTI_DECLARE_NAME_TEMPLATE(TTransform, double);

RTTI_BEGIN_CLASS_DECLARATION_TEMPLATE(TTransform, T)
PROPERTY(Location)
PROPERTY(Rotation)
PROPERTY(Scale)
PROPERTY(bModelMatrixDirty)
PROPERTY(ModelMatrix)
RTTI_END_CLASS_DECLARATION;

using FTransform = TTransform<float>;
using DTransform = TTransform<double>;

#include "Transform.inl"

#pragma once

#include "Engine/Math/Matrix.hxx"
#include "Engine/Math/Quaternion.hxx"
#include "Engine/Math/Vector.hxx"

namespace Math
{

template <typename T>
class TTransform
{
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

    FMatrix4 GetModelMatrix();

    FMatrix4 GetTranslationMatrix() const;
    FMatrix4 GetRotationMatrix() const;
    FMatrix4 GetScaleMatrix() const;

public:
    TVector3<T> Location = {0, 0, 0};
    TQuaternion<T> Rotation = {0, 0, 0, 1};
    TVector3<T> Scale = {1, 1, 1};

    bool bModelMatrixDirty = true;
    FMatrix4 ModelMatrix;
};

}    // namespace Math

using FTransform = Math::TTransform<float>;
using DTransform = Math::TTransform<double>;

#include "Transform.inl"

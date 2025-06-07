namespace Math
{

template <typename T>
TTransform<T>::TTransform(const TVector3<T>& InLocation, const TQuaternion<T>& InRotation, const TVector3<T>& InScale)
    : Location(InLocation)
    , Rotation(InRotation)
    , Scale(InScale)
{
}

template <typename T>
void TTransform<T>::SetLocation(const TVector3<T>& InLocation)
{
    Math::CheckNaN(InLocation);
    Location = InLocation;
    bModelMatrixDirty = true;
}

template <typename T>
void TTransform<T>::SetRotation(const TQuaternion<T>& InRotation)
{
    Math::CheckNaN(InRotation);
    Rotation = InRotation;
    bModelMatrixDirty = true;
}

template <typename T>
void TTransform<T>::SetScale(const TVector3<T>& InScale)
{
    Math::CheckNaN(InScale);
    Scale = InScale;
    bModelMatrixDirty = true;
}

template <typename T>
const TVector3<T>& TTransform<T>::GetLocation() const
{
    return Location;
}

template <typename T>
const TQuaternion<T>& TTransform<T>::GetRotation() const
{
    return Rotation;
}

template <typename T>
const TVector3<T>& TTransform<T>::GetScale() const
{
    return Scale;
}

template <typename T>
TVector3<T>& TTransform<T>::GetLocation()
{
    return Location;
}

template <typename T>
TQuaternion<T>& TTransform<T>::GetRotation()
{
    return Rotation;
}

template <typename T>
TVector3<T>& TTransform<T>::GetScale()
{
    return Scale;
}

template <typename T>
TMatrix4<T> TTransform<T>::GetTranslationMatrix() const
{
    TMatrix4<T> TranslationMatrix = TMatrix4<T>::Identity();

    TranslationMatrix[3, 0] = Location.x;
    TranslationMatrix[3, 1] = Location.y;
    TranslationMatrix[3, 2] = Location.z;
    return TranslationMatrix;
}

template <typename T>
TMatrix4<T> TTransform<T>::GetRotationMatrix() const
{
    return Rotation.GetRotationMatrix();
}

template <typename T>
TMatrix4<T> TTransform<T>::GetScaleMatrix() const
{
    TMatrix4<T> ScaleMatrix = TMatrix4<T>::Identity();
    ScaleMatrix[0, 0] = Scale.x;
    ScaleMatrix[1, 1] = Scale.y;
    ScaleMatrix[2, 2] = Scale.z;
    return ScaleMatrix;
}

template <typename T>
TMatrix4<T> TTransform<T>::GetModelMatrix()
{
    if (!bModelMatrixDirty)
    {
        return ModelMatrix;
    }

    RPH_PROFILE_FUNC()
    ModelMatrix = GetTranslationMatrix() * Rotation.GetRotationMatrix() * GetScaleMatrix();

    Math::CheckNaN(ModelMatrix);
    bModelMatrixDirty = false;
    return ModelMatrix;
}

}    // namespace Math

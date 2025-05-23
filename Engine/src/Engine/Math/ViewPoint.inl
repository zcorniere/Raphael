namespace Math
{

template <typename T>
void TViewPoint<T>::ComputeProjectionMatrix()
{
    ensureAlways(m_AspectRatio != 0.0f);
    const T tanHalfFOV = std::tan(m_FOV / T(2));

    TMatrix4<T> NewProjectionMatrix;
    NewProjectionMatrix[0, 0] = T(1) / (m_AspectRatio * tanHalfFOV);
    NewProjectionMatrix[1, 1] = T(1) / tanHalfFOV;
    NewProjectionMatrix[2, 2] = (m_Far + m_Near) / (m_Near - m_Far);
    NewProjectionMatrix[2, 3] = T(-1);
    NewProjectionMatrix[3, 2] = (T(2) * m_Far * m_Near) / (m_Near - m_Far);

    NewProjectionMatrix[1, 1] *= -1;

    m_ProjectionMatrix = NewProjectionMatrix;
}

template <typename T>
void TViewPoint<T>::ComputeViewMatrix(const TTransform<T>& InTransform)
{
    TMatrix4<T> RotationMatrix = InTransform.GetRotationMatrix();

    TVector4<T> Right = Math::Normalize(RotationMatrix[0]);
    TVector4<T> Forward = Math::Normalize(RotationMatrix[1]);
    TVector4<T> Up = Math::Normalize(RotationMatrix[2]);

    Forward = -Forward;

    // Create the final view matrix
    TMatrix4<T> NewViewMatrix(1.0);
    NewViewMatrix[0] = Right;
    NewViewMatrix[1] = Up;
    NewViewMatrix[2] = Forward;

    TVector4<T> Location = InTransform.GetLocation();
    NewViewMatrix[3, 0] = -Math::Dot(Right, Location);
    NewViewMatrix[3, 1] = -Math::Dot(Up, Location);
    NewViewMatrix[3, 2] = Math::Dot(Forward, Location);
    NewViewMatrix[3, 3] = 1.0;

    m_ViewMatrix = NewViewMatrix;
}

template <typename T>
void CheckNan(const TViewPoint<T>& Value)
{
    ensureAlwaysMsg(!std::isnan(Value.m_FOV), "NaN detected in ViewPoint<{}> FOV", RTTI::TypeName<T>());
    ensureAlwaysMsg(!std::isnan(Value.m_AspectRatio), "NaN detected in ViewPoint<{}> AspectRatio", RTTI::TypeName<T>());
    ensureAlwaysMsg(!std::isnan(Value.m_Near), "NaN detected in ViewPoint<{}> Near", RTTI::TypeName<T>());
    ensureAlwaysMsg(!std::isnan(Value.m_Far), "NaN detected in ViewPoint<{}> Far", RTTI::TypeName<T>());

    CheckNaN(Value.Transform);
    CheckNaN(Value.m_ProjectionMatrix);
    CheckNaN(Value.m_ViewMatrix);
}

}    // namespace Math

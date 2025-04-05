#pragma once

#include "Engine/Math/Math.hxx"
#include "Engine/Math/Transform.hxx"

namespace Math
{

template <typename T>
class TViewPoint
{
public:
    TViewPoint() = default;

    /// @brief Construct a new ViewPoint object
    /// @param FOV The field of view in degrees
    /// @param Near The near plane
    /// @param Far The far plane
    /// @param AspectRatio The aspect ratio of the screen
    TViewPoint(T FOV, T Near, T Far, T AspectRatio = T(0))
        : m_FOV(DegreeToRadian(FOV)), m_AspectRatio(AspectRatio), m_Near(Near), m_Far(Far), bProjectionMatrixDirty(true)
    {
    }

    void SetFOV(T FOV)
    {
        const T NewFOV = DegreeToRadian(FOV);
        if (m_FOV == NewFOV) {
            return;
        }
        m_FOV = NewFOV;
        bProjectionMatrixDirty = true;
    }
    T GetFOV() const
    {
        return m_FOV;
    }
    void SetAspectRatio(T AspectRatio)
    {
        if (m_AspectRatio == AspectRatio) {
            return;
        }
        m_AspectRatio = AspectRatio;
        bProjectionMatrixDirty = true;
    }
    T GetAspectRatio() const
    {
        return m_AspectRatio;
    }
    void SetNear(T Near)
    {
        if (m_Near == Near) {
            return;
        }
        m_Near = Near;
        bProjectionMatrixDirty = true;
    }
    T GetNear() const
    {
        return m_Near;
    }
    void SetFar(T Far)
    {
        if (m_Far == Far) {
            return;
        }
        m_Far = Far;
        bProjectionMatrixDirty = true;
    }
    T GetFar() const
    {
        return m_Far;
    }

    TMatrix4<T> GetProjectionMatrix()
    {
        if (bProjectionMatrixDirty) {
            ComputeProjectionMatrix();
            bProjectionMatrixDirty = false;
        }
        return m_ProjectionMatrix;
    }

    TMatrix4<T> GetViewMatrix(const TTransform<T>& InTransform)
    {
        if (bViewMatrixDirty) {
            ComputeViewMatrix(InTransform);
            bViewMatrixDirty = false;
        }
        return m_ViewMatrix;
    }

    TMatrix4<T> GetViewProjectionMatrix()
    {
        return GetProjectionMatrix() * GetViewMatrix();
    }

private:
    void ComputeProjectionMatrix();
    void ComputeViewMatrix(const TTransform<T>& InTransform);

private:
    /// @brief The field of view in radians
    T m_FOV = 0;

    T m_AspectRatio = 0;
    T m_Near = 0;
    T m_Far = 0;

    bool bProjectionMatrixDirty = true;
    TMatrix4<T> m_ProjectionMatrix;

    bool bViewMatrixDirty = true;
    TMatrix4<T> m_ViewMatrix;

    TTransform<T> Transform;
};

template <typename T>
requires std::is_floating_point_v<T>
void CheckNan(const TViewPoint<T>& Value);

}    // namespace Math

using FViewPoint = Math::TViewPoint<float>;
using DViewPoint = Math::TViewPoint<double>;

#include "ViewPoint.inl"

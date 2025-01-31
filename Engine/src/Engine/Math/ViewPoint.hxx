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
    /// @param AspectRatio The aspect ratio of the screen
    /// @param Near The near plane
    /// @param Far The far plane
    TViewPoint(T FOV, T AspectRatio, T Near, T Far)
        : m_FOV(DegreeToRadian(FOV)), m_AspectRatio(AspectRatio), m_Near(Near), m_Far(Far), bProjectionMatrixDirty(true)
    {
    }

    void SetFOV(T FOV)
    {
        m_FOV = DegreeToRadian(FOV);
        bProjectionMatrixDirty = true;
    }
    T GetFOV() const
    {
        return m_FOV;
    }
    void SetAspectRatio(T AspectRatio)
    {
        m_AspectRatio = AspectRatio;
        bProjectionMatrixDirty = true;
    }
    T GetAspectRatio() const
    {
        return m_AspectRatio;
    }
    void SetNear(T Near)
    {
        m_Near = Near;
        bProjectionMatrixDirty = true;
    }
    T GetNear() const
    {
        return m_Near;
    }
    void SetFar(T Far)
    {
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

    TMatrix4<T> GetViewMatrix()
    {
        if (bViewMatrixDirty) {
            ComputeViewMatrix();
            bViewMatrixDirty = false;
        }
        return m_ViewMatrix;
    }

    TMatrix4<T> GetViewProjectionMatrix()
    {
        return GetProjectionMatrix() * GetViewMatrix();
    }

    void SetLocation(const TVector3<T>& InLocation)
    {
        Transform.SetLocation(InLocation);
        bViewMatrixDirty = true;
    }
    const TVector3<T>& GetLocation() const
    {
        return Transform.GetLocation();
    }

private:
    void ComputeProjectionMatrix();
    void ComputeViewMatrix();

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

}    // namespace Math

using FViewPoint = Math::TViewPoint<float>;
using DViewPoint = Math::TViewPoint<double>;

#include "ViewPoint.inl"

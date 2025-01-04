#pragma once

namespace Math
{

class FViewPoint
{
public:
    FViewPoint(float FOV, float AspectRatio, float Near, float Far)
        : m_FOV(FOV), m_AspectRatio(AspectRatio), m_Near(Near), m_Far(Far), bProjectionMatrixDirty(true)
    {
    }

    void SetFOV(float FOV)
    {
        m_FOV = FOV;
        bProjectionMatrixDirty = true;
    }
    float GetFOV() const
    {
        return m_FOV;
    }
    void SetAspectRatio(float AspectRatio)
    {
        m_AspectRatio = AspectRatio;
        bProjectionMatrixDirty = true;
    }
    float GetAspectRatio() const
    {
        return m_AspectRatio;
    }
    void SetNear(float Near)
    {
        m_Near = Near;
        bProjectionMatrixDirty = true;
    }
    float GetNear() const
    {
        return m_Near;
    }
    void SetFar(float Far)
    {
        m_Far = Far;
        bProjectionMatrixDirty = true;
    }
    float GetFar() const
    {
        return m_Far;
    }

    FMatrix4 GetProjectionMatrix()
    {
        if (bProjectionMatrixDirty) {
            ComputeProjectionMatrix();
            bProjectionMatrixDirty = false;
        }
        return m_ProjectionMatrix;
    }

    void SetLocation(const FVector3& InLocation)
    {
        Location = InLocation;
        bViewMatrixDirty = true;
    }
    const FVector3& GetLocation() const
    {
        return Location;
    }

private:
    void ComputeProjectionMatrix();
    void ComputeViewMatrix();

private:
    float m_FOV = 0;
    float m_AspectRatio = 0;
    float m_Near = 0;
    float m_Far = 0;

    bool bProjectionMatrixDirty = true;
    FMatrix4 m_ProjectionMatrix;

    FVector3 Location = {0, 0, 0};
    FVector3 Rotation = {0, 0, 0};
    bool bViewMatrixDirty = true;
    FMatrix4 m_ViewMatrix;
};

}    // namespace Math

#include "ViewPoint.inl"

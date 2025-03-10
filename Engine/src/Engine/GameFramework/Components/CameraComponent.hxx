#pragma once

#include "Engine/GameFramework/Components/SceneComponent.hxx"
#include "Engine/Math/ViewPoint.hxx"

template <typename T>
class RCameraComponent : public RSceneComponent
{
    RTTI_DECLARE_TYPEINFO(RCameraComponent, RSceneComponent);

public:
    RCameraComponent(T FOV, T Near, T Far, T AspectRatio = T(0));
    ~RCameraComponent() = default;

    void SetFOV(T FOV);
    T GetFOV() const;

    void SetAspectRatio(T AspectRatio);
    T GetAspectRatio() const;

    void SetNearFar(T Near, T Far);
    T GetNear() const;
    T GetFar() const;

    TMatrix4<T> GetViewMatrix() const;
    TMatrix4<T> GetProjectionMatrix() const;

private:
    mutable Math::TViewPoint<T> ViewPoint;
};

template <typename T>
RCameraComponent<T>::RCameraComponent(T FOV, T Near, T Far, T AspectRatio): ViewPoint(FOV, Near, Far, AspectRatio)
{
    MarkTransformDirty();
}

template <typename T>
void RCameraComponent<T>::SetFOV(T FOV)
{
    if (ViewPoint.GetFOV() == FOV) {
        return;
    }
    ViewPoint.SetFOV(FOV);
    MarkRenderStateDirty();
}

template <typename T>
T RCameraComponent<T>::GetFOV() const
{
    return ViewPoint.GetFOV();
}

template <typename T>
void RCameraComponent<T>::SetAspectRatio(T AspectRatio)
{
    if (ViewPoint.GetAspectRatio() == AspectRatio) {
        return;
    }
    ViewPoint.SetAspectRatio(AspectRatio);
    MarkRenderStateDirty();
}

template <typename T>
T RCameraComponent<T>::GetAspectRatio() const
{
    return ViewPoint.GetAspectRatio();
}

template <typename T>
void RCameraComponent<T>::SetNearFar(T Near, T Far)
{
    if (ViewPoint.GetNear() == Near && ViewPoint.GetFar() == Far) {
        return;
    }
    ViewPoint.SetNear(Near);
    ViewPoint.SetFar(Far);
    MarkRenderStateDirty();
}

template <typename T>
T RCameraComponent<T>::GetNear() const
{
    return ViewPoint.GetNear();
}

template <typename T>
T RCameraComponent<T>::GetFar() const
{
    return ViewPoint.GetFar();
}

template <typename T>
TMatrix4<T> RCameraComponent<T>::GetViewMatrix() const
{
    return ViewPoint.GetViewMatrix(GetRelativeTransform());
}

template <typename T>
TMatrix4<T> RCameraComponent<T>::GetProjectionMatrix() const
{
    return ViewPoint.GetProjectionMatrix();
}

#include "Engine/GameFramework/CameraActor.hxx"

ACameraActor::ACameraActor()
{
    CameraComponent = Ref<RCameraComponent<float>>::Create(60.0, 0.1, 1000.0, 16.0 / 9.0);
    CameraComponent->MarkRenderStateDirty();
    SetRootComponent(CameraComponent);
}

ACameraActor::~ACameraActor()
{
}

RObject* ACameraActor::FindComponent(RTTI::FTypeId TypeId)
{
    if (CameraComponent->TypeId() == TypeId)
    {
        return CameraComponent.Raw();
    }

    return AActor::FindComponent(TypeId);
}

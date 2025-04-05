#pragma once

#include "Engine/GameFramework/Actor.hxx"

#include "Engine/GameFramework/Components/CameraComponent.hxx"

class ACameraActor : public AActor
{
    RTTI_DECLARE_TYPEINFO(ACameraActor, AActor)

public:
    ACameraActor();
    virtual ~ACameraActor();

protected:
    RObject* FindComponent(RTTI::FTypeId TypeId) override;

private:
    Ref<RCameraComponent<float>> CameraComponent = nullptr;
};

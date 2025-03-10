#pragma once

#include "Engine/Core/UUID.hxx"

#include "Engine/GameFramework/Components/MeshComponent.hxx"

class AActor : public RObject, public Raphael::FUUID
{
    RTTI_DECLARE_TYPEINFO(AActor, RObject)
public:
    AActor();
    virtual ~AActor();

    virtual void BeginPlay();
    virtual void EndPlay();

    virtual void Tick(double DeltaTime);

    RMeshComponent* GetMesh();

    RSceneComponent* GetRootComponent();
    void SetRootComponent(Ref<RSceneComponent> InRootComponent);

    void SetActorLocation(const FVector3& Location);
    void SetActorRotation(const FQuaternion& Rotation);
    void SetActorScale(const FVector3& Scale);
    void SetActorTransform(const FTransform& Transform);

    const FTransform& GetRelativeTransform() const;

    template <typename T>
    requires RTTI::IsRTTIApiAvailable<T>
    T* GetComponent()
    {
        return static_cast<T*>(FindComponent(T::TypeInfo::Id()));
    }

protected:
    virtual RObject* FindComponent(RTTI::FTypeId TypeId);

private:
    WeakRef<RSceneComponent> RootComponent = nullptr;
    Ref<RMeshComponent> MeshComponent = nullptr;
};

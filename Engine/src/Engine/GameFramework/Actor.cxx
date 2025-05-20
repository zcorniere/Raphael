#include "Engine/GameFramework/Actor.hxx"

IMPLEMENT_RTTI(AActor)

IMPLEMENT_RTTI(RMeshComponent)
IMPLEMENT_RTTI(RSceneComponent)

AActor::AActor()
{
    MeshComponent = Ref<RMeshComponent>::Create();
    SetRootComponent(MeshComponent);
}

AActor::~AActor()
{
}

void AActor::BeginPlay()
{
}

void AActor::EndPlay()
{
}

void AActor::Tick(double DeltaTime)
{
    (void)DeltaTime;
}

RMeshComponent* AActor::GetMesh()
{
    return MeshComponent.Raw();
}

RSceneComponent* AActor::GetRootComponent()
{
    return RootComponent.Raw();
}

void AActor::SetRootComponent(Ref<RSceneComponent> InRootComponent)
{
    RootComponent = InRootComponent;
    RootComponent->MarkTransformDirty();
}

RObject* AActor::FindComponent(RTTI::FTypeId TypeId)
{
    if (MeshComponent->TypeId() == TypeId) {
        return MeshComponent.Raw();
    }

    return nullptr;
}

void AActor::SetActorLocation(const FVector3& Location)
{
    GetRootComponent()->SetRelativeLocation(Location);
}

void AActor::SetActorRotation(const FQuaternion& Rotation)
{
    GetRootComponent()->SetRelativeRotation(Rotation);
}

void AActor::SetActorScale(const FVector3& Scale)
{
    GetRootComponent()->SetRelativeScale(Scale);
}

void AActor::SetActorTransform(const FTransform& Transform)
{
    GetRootComponent()->SetRelativeTransform(Transform);
}

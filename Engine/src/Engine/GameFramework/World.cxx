#include "Engine/GameFramework/World.hxx"

#include "Engine/Core/Engine.hxx"
#include "Engine/Core/RHI/GenericRHI.hxx"
#include "Engine/Core/RHI/RHIScene.hxx"

#include "Engine/GameFramework/Actor.hxx"

RWorld::RWorld()
{
    Scene = Ref<RRHIScene>::Create(this);
    RHI::Get()->RegisterScene(Scene);
}

RWorld::~RWorld()
{
    RHI::Get()->UnregisterScene(Scene);
}

void RWorld::SetName(std::string_view InName)
{
    Super::SetName(InName);
    Scene->SetName(std::format("{:s} Scene", InName));
}

void RWorld::AddToWorld(Ref<AActor> Actor)
{
    Actors.Add(Actor);
    OnActorAddedToWorld.Broadcast(Actor.Raw());
}

void RWorld::RemoveFromWorld(Ref<AActor> Actor)
{
    OnActorRemovedFromWorld.Broadcast(Actor.Raw());
    Actors.Remove(Actor);
}

void RWorld::Tick(double DeltaTime)
{
    RPH_PROFILE_FUNC();

    {
        RPH_PROFILE_FUNC("Actor Tick - parallel");
        std::shared_ptr<std::latch> Latch =
            GEngine->GetThreadPool().ParallelFor(Actors.Size(), [this, DeltaTime](unsigned i) {
                Ref<AActor>& Actor = Actors[i];
                HandleActorTick(Actor.Raw(), DeltaTime);
            });
        Latch->wait();
    }

    Scene->Tick(DeltaTime);
}

void RWorld::HandleActorTick(AActor* const Actor, double DeltaTime)
{
    RPH_PROFILE_FUNC();

    Actor->Tick(DeltaTime);

    RSceneComponent* const RootComponent = Actor->GetRootComponent();
    if (RootComponent->IsTransformDirty()) {
        const FTransform RelativeTransform = RootComponent->GetRelativeTransform();
        Scene->UpdateActorLocation(Actor->ID(), RelativeTransform);
        RootComponent->ClearDirtyTransformFlag();
    }
}

Ref<RRHIScene> RWorld::GetScene() const
{
    return Scene;
}

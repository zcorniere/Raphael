#include "Engine/Core/ECS/ECS.hxx"

#include "Engine/Core/ECS/World.hxx"
#include "Engine/Core/Engine.hxx"

namespace ecs
{

Ref<RWorld> CreateWorld()
{
    Ref<RWorld> NewWorld = Ref<RWorld>::Create();

    NewWorld->RegisterComponent<ecs::FTransformComponent>();
    NewWorld->RegisterComponent<ecs::FMeshComponent>();
    NewWorld->RegisterComponent<ecs::FCameraComponent>();
    NewWorld->RegisterComponent<ecs::FRenderTargetComponent>();

    // Register the basic systems
    FSystem CollectRenderablesSystem(NewWorld->GetScene(), &RHIScene::CollectRenderablesSystem);
    NewWorld->RegisterSystem(std::move(CollectRenderablesSystem));

    FSystem CameraSystem(NewWorld->GetScene(), &RHIScene::CameraSystem);
    NewWorld->RegisterSystem(std::move(CameraSystem));

    FSystem CollectRenderTargets(NewWorld->GetScene(), &RHIScene::CollectRenderTargets);
    NewWorld->RegisterSystem(std::move(CollectRenderTargets));

    return NewWorld;
}

void DestroyWorld(Ref<RWorld>& World)
{
    if (GEngine->GetWorld() == World) {
        GEngine->SetWorld(nullptr);
    }
    check(World->GetRefCount() == 1);
    World = nullptr;
}

}    // namespace ecs

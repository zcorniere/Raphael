#include "Engine/Core/ECS/ECS.hxx"

#include "Engine/Core/ECS/World.hxx"
#include "Engine/Core/Engine.hxx"

namespace ecs
{

Ref<RWorld> CreateWorld()
{
    Ref<RWorld> NewWorld = Ref<RWorld>::Create();

    // Register the basic systems
    FSystem CollectRenderablesSystem(NewWorld->GetScene(), &RHIScene::CollectRenderablesSystem);
    NewWorld->RegisterSystem(std::move(CollectRenderablesSystem));

    FSystem CameraSystem(NewWorld->GetScene(), &RHIScene::CameraSystem);
    NewWorld->RegisterSystem(std::move(CameraSystem));

    return NewWorld;
}

void DestroyWorld(Ref<RWorld>& World)
{
    if (GEngine->GetWorld() == World) {
        GEngine->SetWorld(nullptr);
    }
    World = nullptr;
}

}    // namespace ecs

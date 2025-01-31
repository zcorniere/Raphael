#include "Engine/Core/ECS/ECS.hxx"

#include "Engine/Core/ECS/World.hxx"
#include "Engine/Core/Engine.hxx"

namespace ecs
{

Ref<RWorld> CreateWorld()
{
    Ref<RWorld> NewWorld = Ref<RWorld>::Create();
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

#include "Engine/Core/ECS/World.hxx"

#include "Engine/Core/ECS/System.hxx"

namespace ecs
{

RWorld::RWorld()
{
}

RWorld::~RWorld()
{
}

void RWorld::Update(float DeltaTime)
{
    Scheduler.Update(this);
}

FEntityBuilder RWorld::CreateEntity()
{
    return Storage.BuildEntity();
}

void RWorld::DestroyEntity(FEntity EntityID)
{
    Storage.DestroyEntity(EntityID);
}

void RWorld::RegisterSystem(FSystem system)
{
    Scheduler.AddSystem(system);
}

}    // namespace ecs

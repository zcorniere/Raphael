#include "Engine/Core/ECS/World.hxx"

#include "Engine/Core/ECS/System.hxx"
#include "Engine/Core/RHI/GenericRHI.hxx"

namespace ecs
{

RWorld::RWorld()
{
    Scene = Ref<RHIScene>::Create();
    RHI::Get()->RegisterScene(Scene);
}

RWorld::~RWorld()
{
}

void RWorld::Update(float DeltaTime)
{
    fDeltaTime = DeltaTime;

    Scheduler.Update(this);
    Scene->Tick(DeltaTime);
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

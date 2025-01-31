#include "Engine/Core/ECS/SystemScheduler.hxx"

#include "Engine/Core/ECS/World.hxx"

#include "Engine/Core/ECS/System.hxx"

void ecs::FSystem::Call(RWorld* world) const
{
    CallWrapper(world);
}

void ecs::FSystemScheduler::AddSystem(FSystem system)
{
    SystemStorage.Add(std::move(system));
}

void ecs::FSystemScheduler::Update(RWorld* world)
{
    for (FSystem& system: SystemStorage) {
        system.Call(world);
    }
}

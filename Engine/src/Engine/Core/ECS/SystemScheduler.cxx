#include "Engine/Core/ECS/SystemScheduler.hxx"

#include "Engine/Core/ECS/World.hxx"

void ecs::System::Call(World* world) const
{
    CallWrapper(world);
}

void ecs::SystemScheduler::AddSystem(System system)
{
    SystemStorage.Add(system);
}

void ecs::SystemScheduler::Update(World* world)
{
    for (System& system: SystemStorage) {
        system.Call(world);
    }
}

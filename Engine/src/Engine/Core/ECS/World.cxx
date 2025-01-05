#include "Engine/Core/ECS/World.hxx"

namespace ecs
{

World::World()
{
}

World::~World()
{
}

void World::Update(float DeltaTime)
{
    Scheduler.Update(this);
}

}    // namespace ecs

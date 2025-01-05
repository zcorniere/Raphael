#include "Engine/Core/ECS/ComponentStorage.hxx"

ecs::EntityBuilder ecs::ComponentStorage::BuildEntity()
{
    return EntityBuilder(*this);
}

void ecs::ComponentStorage::DestroyEntity(Entity EntityID)
{
    DeadEntities.Add(EntityID);

    for (auto& [_, ComponentArray]: ComponentArrays) {
        ComponentArray->Remove(EntityID);
    }
}

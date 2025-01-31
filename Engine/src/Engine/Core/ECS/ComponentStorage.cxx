#include "Engine/Core/ECS/ComponentStorage.hxx"

ecs::FEntityBuilder ecs::FComponentStorage::BuildEntity()
{
    return FEntityBuilder(*this);
}

void ecs::FComponentStorage::DestroyEntity(FEntity EntityID)
{
    DeadEntities.Add(EntityID);

    for (auto& [_, ComponentArray]: ComponentArrays) {
        ComponentArray->Remove(EntityID);
    }
}

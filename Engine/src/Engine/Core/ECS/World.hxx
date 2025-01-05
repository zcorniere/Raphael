#pragma once

#include "Engine/Core/ECS/ComponentStorage.hxx"
#include "Engine/Core/ECS/SystemScheduler.hxx"

namespace ecs
{

class World : public RObject
{
public:
    World();
    ~World();

    void Update(float DeltaTime);

    ComponentStorage& GetComponentStorage()
    {
        return Storage;
    }

private:
    SystemScheduler Scheduler;
    ComponentStorage Storage;
};

}    // namespace ecs

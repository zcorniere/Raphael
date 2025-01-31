#pragma once

#include "Engine/Core/ECS/ECS.hxx"

#include "Engine/Core/ECS/ComponentStorage.hxx"
#include "Engine/Core/ECS/SystemScheduler.hxx"
#include "Engine/Core/RHI/Resources/RHIViewport.hxx"

namespace ecs
{

class RWorld : public RObject
{
    RTTI_DECLARE_TYPEINFO(RWorld, RObject);

public:
    RWorld();
    ~RWorld();

    void Update(float DeltaTime);

    FEntityBuilder CreateEntity();
    void DestroyEntity(FEntity EntityID);

    template <typename T>
    void RegisterComponent()
    {
        Storage.RegisterComponent<T>();
    }

    void RegisterSystem(FSystem system);

    FSystemScheduler& GetScheduler()
    {
        return Scheduler;
    }

    FComponentStorage& GetComponentStorage()
    {
        return Storage;
    }

public:
    WeakRef<RRHIViewport> Viewport;

private:
    FSystemScheduler Scheduler;
    FComponentStorage Storage;
};

}    // namespace ecs

#include "Engine/Core/ECS/System.hxx"

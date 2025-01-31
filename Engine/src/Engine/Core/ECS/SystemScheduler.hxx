#pragma once

namespace ecs
{

class RWorld;
class FSystem;

class FSystemScheduler
{
public:
    void AddSystem(FSystem system);

    void Update(RWorld* world);

private:
    TArray<FSystem> SystemStorage;
};

}    // namespace ecs

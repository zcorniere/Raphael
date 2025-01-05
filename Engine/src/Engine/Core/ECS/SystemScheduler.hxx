#pragma once

#include "Engine/Core/ECS/details/function_traits.hpp"

namespace ecs
{

class World;

template <typename Fn>
concept SystemFunction =
    std::is_same_v<typename function_traits<Fn>::result_type, void> && function_traits<Fn>::arity > 0;

class System
{
public:
    System() = default;

    System(std::function<void(World*)> system);

    template <typename Fn>
    requires(!std::is_same_v<System, std::decay_t<Fn>>) && SystemFunction<Fn>
    System(Fn system);

    void Call(World*) const;

private:
    std::function<void(World*)> CallWrapper;
};

class SystemScheduler
{
public:
    void AddSystem(System system);

    void Update(World* world);

private:
    TArray<System> SystemStorage;
};

}    // namespace ecs

// #include "details/System.inl"

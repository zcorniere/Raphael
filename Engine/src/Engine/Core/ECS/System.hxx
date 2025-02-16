#pragma once

#include "Engine/Core/ECS/details/function_traits.hpp"

#include "Engine/Core/ECS/World.hxx"

namespace ecs
{

namespace details
{

    template <typename T>
    using cleaned_component = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

    template <typename T>
    struct wrapper : public wrapper<typename function_traits<T>::args_type> {
    };

    template <typename... Components>
    struct wrapper<std::tuple<Components...>> {
        static constexpr auto wrap_system(auto system)
        {
            return [system](ecs::RWorld* World) {
                TComponentArray components_map =
                    World->GetComponentStorage().JoinComponents<cleaned_component<Components>...>();

                for (auto& [Entity, components]: components_map)
                    std::apply(system, components);
            };
        }

        template <typename TClass>
        static constexpr auto wrap_system(TClass* InContext, auto system)
        {
            return [InContext, system](ecs::RWorld* World) {
                TComponentArray components_map =
                    World->GetComponentStorage().JoinComponents<cleaned_component<Components>...>();

                for (auto& [Entity, components]: components_map) {
                    std::apply(std::bind_front(system, InContext), components);
                }
            };
        }
    };

}    // namespace details

template <typename Fn>
concept SystemFunction =
    std::is_same_v<typename function_traits<Fn>::result_type, void> && function_traits<Fn>::arity > 0;

class FSystem
{
public:
    FSystem(std::function<void(RWorld*)> system);

    template <typename Fn>
    requires(!std::is_same_v<FSystem, std::decay_t<Fn>> && SystemFunction<Fn>)
    FSystem(Fn system): CallWrapper(details::wrapper<decltype(system)>::wrap_system(system))
    {
    }

    template <typename TClass, typename Fn>
    requires(!std::is_same_v<FSystem, std::decay_t<Fn>> && SystemFunction<Fn> &&
             std::is_same_v<typename function_traits<Fn>::Class, TClass>)
    /// Call context for the system (basically the this pointer for member functions systems)
    FSystem(TClass* InContext, Fn system)
        : CallWrapper(details::wrapper<decltype(system)>::template wrap_system<TClass>(InContext, system))
    {
    }

    void Call(RWorld*) const;

private:
    std::function<void(RWorld*)> CallWrapper;
};

}    // namespace ecs

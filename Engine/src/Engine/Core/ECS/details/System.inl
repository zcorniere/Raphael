#pragma once

#include <tuple>

#include "Engine/Core/ECS/World.hxx"
#include "Engine/Core/ECS/details/function_traits.hpp"

namespace ecs
{

namespace details
{

    template <typename T>
    using cleaned_component = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

    template <typename T, typename M>
    struct split_modules_components;

    template <typename... Modules, typename First, typename... Rest>
    requires std::is_same_v<World, cleaned_component<First>>
    struct split_modules_components<std::tuple<Modules...>, std::tuple<First, Rest...>>
        : public split_modules_components<std::tuple<Modules..., First>, std::tuple<Rest...>> {
    };

    template <typename... Modules, typename... Components>
    struct split_modules_components<std::tuple<Modules...>, std::tuple<Components...>> {
        using modules = std::tuple<Modules...>;
        using components = std::tuple<Components...>;
        using type = std::pair<modules, components>;
    };
    template <typename T>
    struct wrapper
        : public wrapper<
              typename split_modules_components<std::tuple<>, typename function_traits<T>::args_type>::type> {
    };

    template <typename... Components>
    struct wrapper<std::tuple<Components...>> {
        static constexpr auto wrap_system(auto system)
        {
            return [system](ecs::World* World) {
                auto components_map = World->GetComponentStorage().JoinComponents(
                    World->GetComponentStorage().GetComponentArray<cleaned_component<Components>>()...);

                for (auto& [_, components]: components_map)
                    std::apply(system, std::tuple_cat(components));
            };
        }
    };

}    // namespace details

template <SystemFunction Fn>
System::System(Fn system): CallWrapper(details::wrapper<decltype(system)>::wrap_system(system))
{
}

}    // namespace ecs

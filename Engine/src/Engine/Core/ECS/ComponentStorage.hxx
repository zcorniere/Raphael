#pragma once

#include <typeindex>

#include "Engine/Core/ECS/ECS.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogComponentStorage, Info)

namespace ecs
{

class ComponentStorage;

// This Type is used to hide the template
class IComponentEntityMap
{
public:
    virtual ~IComponentEntityMap() = default;

    virtual void Remove(Entity EntityID) = 0;
};

/// This class is used to store the component data in relation of the entity id that uses it
template <typename T>
class ComponentEntityMap : public IComponentEntityMap
{
public:
    virtual ~ComponentEntityMap() = default;

    virtual void Remove(Entity EntityID) override final
    {
        auto it = Map.find(EntityID);
        if (it == Map.end()) {
            return;
        }

        ComponentsArray.RemoveAt(it->second);
        Map.erase(it);
    }

    void Add(Entity EntityID, T Component)
    {
        ComponentsArray.Emplace(std::move(Component));
        Map[EntityID] = ComponentsArray.Size() - 1;
    }

    T* Get(Entity EntityID)
    {
        auto it = Map.find(EntityID);
        if (it == Map.end()) {
            return nullptr;
        }
        return &ComponentsArray[it->second];
    }
    const T* Get(Entity EntityID) const
    {
        auto it = Map.find(EntityID);
        if (it == Map.end()) {
            return nullptr;
        }
        return &ComponentsArray[it->second];
    }

    const TArray<T>& GetArray() const
    {
        return ComponentsArray;
    }
    TArray<T>& GetArray()
    {
        return ComponentsArray;
    }

    std::unordered_map<Entity, T&> GetMap()
    {
        std::unordered_map<Entity, T&> result;
        for (auto& [EntityID, Index]: Map) {
            result[EntityID] = ComponentsArray[Index];
        }
        return result;
    }

private:
    TArray<T> ComponentsArray;
    std::unordered_map<Entity, typename decltype(ComponentsArray)::TSize> Map;
};

class EntityBuilder;
class ComponentStorage
{
public:
    ComponentStorage() = default;

    template <typename T>
    requires RTTI::IsRTTIEnabled<T>
    void RegisterComponent()
    {
        ComponentArrays[typeid(T)] = std::make_unique<ComponentEntityMap<T>>();
    }

    template <typename T>
    requires RTTI::IsRTTIEnabled<T>
    void StoreComponent(Entity EntityID, T Component)
    {
        ComponentEntityMap<T>* Array = GetComponentArray<T>();
        if (Array == nullptr) {
            return;
        }
        Array->Add(EntityID, std::move(Component));
    }

    template <typename T>
    requires RTTI::IsRTTIEnabled<T>
    ComponentEntityMap<T>* GetComponentArray()
    {
        std::unordered_map<std::type_index, std::unique_ptr<IComponentEntityMap>>::iterator it =
            ComponentArrays.find(typeid(T));
        if (it == ComponentArrays.end()) {
            LOG(LogComponentStorage, Error, "Component not registered before use");
            return nullptr;
        }
        return static_cast<ComponentEntityMap<T>*>(it->second.get());
    }

    template <typename FirstType, typename... RestTypes>
    std::unordered_map<unsigned int, std::tuple<FirstType&, RestTypes&...>>
    JoinComponents(ComponentEntityMap<FirstType>* FirstComponents, ComponentEntityMap<RestTypes>*... RestOfComponents)
    {
        std::unordered_map<unsigned int, std::tuple<FirstType&, RestTypes&...>> result;

        if (FirstComponents == nullptr) {
            return result;
        }
        for (const auto& [EntityID, Component]: FirstComponents->GetMap()) {
            std::tuple<FirstType&, RestTypes&...> Tuple{Component, RestOfComponents->Get(EntityID)...};
            result[EntityID] = Tuple;
        }
        return result;
    }

    EntityBuilder BuildEntity();
    void DestroyEntity(Entity EntityID);

private:
    Entity GetNewEntityID()
    {
        return DeadEntities.IsEmpty() ? NextEntityID++ : DeadEntities.Pop();
    }

private:
    uint64_t NextEntityID = 0;
    TArray<Entity> DeadEntities;
    std::unordered_map<std::type_index, std::unique_ptr<IComponentEntityMap>> ComponentArrays;

    friend class EntityBuilder;
};

class EntityBuilder
{
public:
    EntityBuilder(ComponentStorage& InStorage): Storage(InStorage)
    {
        EntityID = Storage.GetNewEntityID();
    }

    Entity Build() const
    {
        return EntityID;
    }

    template <typename T>
    requires RTTI::IsRTTIEnabled<T>
    EntityBuilder& WithComponent(T Component)
    {
        Storage.StoreComponent<T>(EntityID, Component);
        return *this;
    }

private:
    Entity EntityID;
    ComponentStorage& Storage;
};

}    // namespace ecs

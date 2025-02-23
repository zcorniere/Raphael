#pragma once

#include "Engine/Core/ECS/ECS.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogComponentStorage, Info)

namespace ecs
{

class FComponentStorage;

// This Type is used to hide the template
class IComponentEntityMap
{
public:
    virtual ~IComponentEntityMap() = default;

    virtual void Remove(FEntity EntityID) = 0;
};

template <RTTI::IsRTTIApiAvailable FirstType, RTTI::IsRTTIApiAvailable... RestTypes>
using TComponentArray = TArray<std::pair<FEntity, std::tuple<FirstType&, RestTypes&...>>>;

/// This class is used to store the component data in relation of the entity id that uses it
template <RTTI::IsRTTIApiAvailable T>
class TEntityComponentMap : public IComponentEntityMap
{
public:
    virtual ~TEntityComponentMap() = default;

    virtual void Remove(FEntity EntityID) override final
    {
        auto it = Map.find(EntityID);
        if (it == Map.end()) {
            return;
        }

        ComponentsArray.RemoveAt(it->second);
        Map.erase(it);
    }

    void Add(FEntity EntityID, T Component)
    {
        ComponentsArray.Emplace(std::move(Component));
        Map[EntityID] = ComponentsArray.Size() - 1;
    }

    T& Get(FEntity EntityID)
    {
        auto it = Map.find(EntityID);
        check(it != Map.end());
        return ComponentsArray[it->second];
    }
    const T& Get(FEntity EntityID) const
    {
        auto it = Map.find(EntityID);
        check(it != Map.end());
        return ComponentsArray[it->second];
    }

    const TArray<T>& GetArray() const
    {
        return ComponentsArray;
    }
    TArray<T>& GetArray()
    {
        return ComponentsArray;
    }

    std::unordered_map<FEntity, T*> GetMap()
    {
        std::unordered_map<FEntity, T*> result;
        for (const auto& [EntityID, Index]: Map) {
            result[EntityID] = &ComponentsArray[Index];
        }
        return result;
    }

private:
    TArray<T> ComponentsArray;
    std::unordered_map<FEntity, typename decltype(ComponentsArray)::TSize> Map;
};

class FEntityBuilder;
class FComponentStorage
{
public:
    FComponentStorage() = default;

    template <RTTI::IsRTTIApiAvailable T>
    void RegisterComponent()
    {
        ComponentArrays[T::TypeInfo::Id()] = std::make_unique<TEntityComponentMap<T>>();
    }

    template <RTTI::IsRTTIApiAvailable T>
    void StoreComponent(FEntity EntityID, T Component)
    {
        TEntityComponentMap<T>* Array = GetComponentArray<T>();
        check(Array);
        Array->Add(EntityID, std::move(Component));
    }

    template <RTTI::IsRTTIApiAvailable T>
    TEntityComponentMap<T>* GetComponentArray()
    {
        decltype(ComponentArrays)::iterator it = ComponentArrays.find(T::TypeInfo::Id());

        if (it == ComponentArrays.end()) {
            LOG(LogComponentStorage, Error, "Component type {} not registered before use!", T::TypeInfo::Name());
            RegisterComponent<T>();
            return GetComponentArray<T>();
        }
        return static_cast<TEntityComponentMap<T>*>(it->second.get());
    }

    template <RTTI::IsRTTIApiAvailable FirstType, RTTI::IsRTTIApiAvailable... RestTypes>
    TComponentArray<FirstType, RestTypes...> JoinComponents()
    {
        TComponentArray<FirstType, RestTypes...> Result;

        TEntityComponentMap<FirstType>* const FirstComponents = GetComponentArray<FirstType>();
        check(FirstComponents);

        Result.Reserve(FirstComponents->GetArray().Size());
        for (const auto& [EntityID, Component]: FirstComponents->GetMap()) {
            std::tuple<FirstType&, RestTypes&...> Tuple{*Component, GetComponentArray<RestTypes>()->Get(EntityID)...};
            Result.Add(std::make_pair(EntityID, Tuple));
        }
        return Result;
    }

    FEntityBuilder BuildEntity();
    void DestroyEntity(FEntity EntityID);

private:
    FEntity GetNewEntityID()
    {
        return DeadEntities.IsEmpty() ? NextEntityID++ : DeadEntities.Pop();
    }

private:
    uint64_t NextEntityID = 0;
    TArray<FEntity> DeadEntities;
    std::unordered_map<RTTI::FTypeId, std::unique_ptr<IComponentEntityMap>> ComponentArrays;

    friend class FEntityBuilder;
};

class FEntityBuilder
{
public:
    FEntityBuilder(FComponentStorage& InStorage): Storage(InStorage)
    {
        EntityID = Storage.GetNewEntityID();
    }

    FEntity Build() const
    {
        return EntityID;
    }

    template <typename T>
    FEntityBuilder& WithComponent(T Component)
    {
        Storage.StoreComponent<T>(EntityID, Component);
        return *this;
    }

private:
    FEntity EntityID;
    FComponentStorage& Storage;
};

}    // namespace ecs

#include "Engine/Core/RTTI/Name.hxx"

namespace RTTI
{
FName::FName(): Index(FNamePool::InvalidIndex)
{
}
FName::FName(const std::string& name)
{
    Set(name);
}

void FName::Set(const std::string& name)
{
    if (name.empty()) {
        Index = FNamePool::InvalidIndex;
        return;
    }

    Index = FNamePool::Get().AddDynamicName(name);
    Name = FNamePool::Get().FindName(Index).c_str();
}

const std::string& FNamePool::FindName(uint32 Index) const
{
    checkMsg(Index < NamePool.Size(), "Index is out of bounds");
    return NamePool[Index].Name;
}

void FNamePool::AddStaticName(FNameHolder name)
{
    std::lock_guard<std::mutex> lock(Mutex);
    ensureAlwaysMsg(NamePool.Size() <= MaxNamePoolSize, "Name pool is full !");
    name.Index = NamePool.Size();
    NamePool.Add(std::move(name));
}

uint32 FNamePool::AddDynamicName(const std::string& Name)
{
    std::lock_guard<std::mutex> lock(Mutex);
    ensureAlwaysMsg(NamePool.Size() <= MaxNamePoolSize, "Name pool is full !");

    // Check if the name already exists
    FNameHolder* Existing = NamePool.FindByLambda([&Name](const FNameHolder& other) { return other.Name == Name; });
    if (Existing) {
        return Existing->Index;
    }

    FNameHolder NameHolder(Name, NamePool.Size());
    NamePool.Add(std::move(NameHolder));
    return NamePool.Back().Index;
}
}    // namespace RTTI

#pragma once

#include "Engine/Containers/Array.hxx"

#include "Engine/Core/RTTI/Name.hxx"

namespace RTTI
{

class IType
{
public:
    virtual ~IType() = default;

    [[nodiscard]] virtual FName GetName() const = 0;
    [[nodiscard]] virtual uint32 GetSize() const = 0;
    [[nodiscard]] virtual uint32 GetAlignment() const = 0;
};

class FClass;
struct FProperty {
    FProperty(FClass* OwnerClass, FName TypeName, FName Name, uint32 Offset, uint32 Alignment);

    FClass* const OwnerClass = nullptr;
    IType* const Type = nullptr;
    const FName Name;
    const uint32 Offset;
    const uint32 Alignment;
};

class FClass : public IType
{
public:
    FClass(FName name, uint32 size, uint32 alignment): Name(std::move(name)), Size(size), Alignment(alignment)
    {
    }

    [[nodiscard]] FName GetName() const override
    {
        return Name;
    }

    [[nodiscard]] uint32 GetSize() const override
    {
        return Size;
    }

    [[nodiscard]] uint32 GetAlignment() const override
    {
        return Alignment;
    }

    void AddProperty(FProperty&& property)
    {
        Properties.Add(std::move(property));
    }

    void AddParentClass(FClass* InParentClass)
    {
        ParentClass.AddUnique(InParentClass);
    }
    [[nodiscard]] const TArray<FClass*>& GetParentClass() const
    {
        return ParentClass;
    }

    [[nodiscard]] const TArray<FProperty>& GetProperties() const
    {
        return Properties;
    }

private:
    TArray<FClass*> ParentClass;

    const FName Name;
    const uint32 Size = 0;
    const uint32 Alignment = 0;
    TArray<FProperty> Properties;
};

template <typename T>
class TTypedClass : public FClass
{
public:
    TTypedClass(FName Name): FClass(std::move(Name), sizeof(T), alignof(T))
    {
    }
};

}    // namespace RTTI

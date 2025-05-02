#pragma once

namespace RTTI
{

class IType
{
public:
    virtual ~IType() = default;

    [[nodiscard]] virtual std::string_view GetName() const = 0;
    [[nodiscard]] virtual uint32 GetSize() const = 0;
};

struct FProperty {
    IType* const Type = nullptr;
    const std::string Name;
    const uint32 Offset;
    const uint32 Alignment;
};

class FClass : public IType
{
public:
    FClass(std::string name, uint32 size, uint32 alignment): Name(std::move(name)), Size(size), Alignment(alignment)
    {
    }

    [[nodiscard]] std::string_view GetName() const override
    {
        return Name;
    }

    [[nodiscard]] uint32 GetSize() const override
    {
        return Size;
    }

    [[nodiscard]] uint32 GetAlignment() const
    {
        return Alignment;
    }

private:
    const std::string Name;
    const uint32 Size = 0;
    const uint32 Alignment = 0;
    TArray<FProperty> Properties;
};

}    // namespace RTTI

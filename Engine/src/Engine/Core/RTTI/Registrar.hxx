#pragma once

#include "Engine/Core/RTTI/RTTITypes.hxx"

namespace RTTI
{

class Registrar
{
public:
    static Registrar& Get()
    {
        static Registrar instance;
        return instance;
    }

public:
    Registrar()
    {
        // We should only have one
        checkNoReentry();
    }

    /// Registers a type in the RTTI system.
    /// @param type The type to register.
    void RegisterType(IType* type);
    /// Unregisters a type from the RTTI system.
    /// @param type The type to unregister.
    void UnregisterType(IType* type);

    /// Finds a type by name.
    /// @param name The name of the type to find.
    /// @return The type if found, nullptr otherwise.
    [[nodiscard]] IType* FindType(std::string_view name) const;

private:
    TArray<IType*> RegisteredTypes;
};

template <typename TType, uint32 Alignment = alignof(TType)>
class TFundamentalType : public IType
{
public:
    TFundamentalType()
    {
        Registrar::Get().RegisterType(this);
    }

    ~TFundamentalType()
    {
        Registrar::Get().UnregisterType(this);
    }

    [[nodiscard]] uint32 GetSize() const override
    {
        return sizeof(TType);
    }
    [[nodiscard]] uint32 GetAlignment() const
    {
        return Alignment;
    }
};

#define DECLARE_PRIMITIVE_RTTI_TYPE(Type)                      \
    class TPrimitiveType##Type : public TFundamentalType<Type> \
    {                                                          \
    public:                                                    \
        std::string_view GetName() const override              \
        {                                                      \
            return #Type;                                      \
        }                                                      \
    };

DECLARE_PRIMITIVE_RTTI_TYPE(bool)
DECLARE_PRIMITIVE_RTTI_TYPE(int8)
DECLARE_PRIMITIVE_RTTI_TYPE(int16)
DECLARE_PRIMITIVE_RTTI_TYPE(int32)
DECLARE_PRIMITIVE_RTTI_TYPE(int64)
DECLARE_PRIMITIVE_RTTI_TYPE(uint8)
DECLARE_PRIMITIVE_RTTI_TYPE(uint16)
DECLARE_PRIMITIVE_RTTI_TYPE(uint32)
DECLARE_PRIMITIVE_RTTI_TYPE(uint64)
DECLARE_PRIMITIVE_RTTI_TYPE(float)
DECLARE_PRIMITIVE_RTTI_TYPE(double)

#undef DECLARE_PRIMITIVE_RTTI_TYPE

}    // namespace RTTI

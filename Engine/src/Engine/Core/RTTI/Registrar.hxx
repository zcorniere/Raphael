#pragma once

#include "Engine/Core/RTTI/RTTITypes.hxx"

#include "Engine/Core/RTTI/Name.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogRegistrar, Trace)

namespace RTTI
{
class IClassBuilder;

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

    void Init();

    void RegisteredClassBuilder(IClassBuilder* classBuilder);
    void UnregisterClassBuilder(IClassBuilder* classBuilder);

    /// Registers a type in the RTTI system.
    /// @param type The type to register.
    void RegisterType(IType* type);
    /// Unregisters a type from the RTTI system.
    /// @param type The type to unregister.
    void UnregisterType(IType* type);

    /// Finds a type by name.
    /// @param name The name of the type to find.
    /// @return The type if found, nullptr otherwise.
    [[nodiscard]] IType* FindType(const FName& name) const;

private:
    TArray<IClassBuilder*> RegisteredClassBuilders;
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

    virtual ~TFundamentalType()
    {
        Registrar::Get().UnregisterType(this);
    }

    [[nodiscard]] uint32 GetSize() const override
    {
        return sizeof(TType);
    }
    [[nodiscard]] uint32 GetAlignment() const override
    {
        return Alignment;
    }
};

}    // namespace RTTI

#define DECLARE_PRIMITIVE_RTTI_TYPE(Type)                          \
    RTTI_DECLARE_NAME(Type);                                       \
    namespace RTTI                                                 \
    {                                                              \
        class TPrimitiveType##Type : public TFundamentalType<Type> \
        {                                                          \
        public:                                                    \
            virtual ~TPrimitiveType##Type() = default;             \
                                                                   \
            virtual FName GetName() const override final           \
            {                                                      \
                return ::RTTI::Names::name_##Type;                 \
            }                                                      \
        };                                                         \
    }

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

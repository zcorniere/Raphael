#include "Engine/Core/RTTI/Registrar.hxx"

namespace RTTI
{
#define DECLARE_PRIMITIVE_RTTI_REGISTRATION(Type) const TPrimitiveType##Type gRegistrator##Type##Registrator;

DECLARE_PRIMITIVE_RTTI_REGISTRATION(bool)
DECLARE_PRIMITIVE_RTTI_REGISTRATION(int8)
DECLARE_PRIMITIVE_RTTI_REGISTRATION(int16)
DECLARE_PRIMITIVE_RTTI_REGISTRATION(int32)
DECLARE_PRIMITIVE_RTTI_REGISTRATION(int64)
DECLARE_PRIMITIVE_RTTI_REGISTRATION(uint8)
DECLARE_PRIMITIVE_RTTI_REGISTRATION(uint16)
DECLARE_PRIMITIVE_RTTI_REGISTRATION(uint32)
DECLARE_PRIMITIVE_RTTI_REGISTRATION(uint64)
DECLARE_PRIMITIVE_RTTI_REGISTRATION(float)
DECLARE_PRIMITIVE_RTTI_REGISTRATION(double)

#undef DECLARE_PRIMITIVE_RTTI_REGISTRATION

DECLARE_LOGGER_CATEGORY(Core, LogRegistrar, Trace)

void Registrar::Init()
{
    LOG(LogRegistrar, Trace, "Registering RTTI types");
    // Register the primitive types
    for (IClassBuilder* classBuilder: RegisteredClassBuilders) {
        FClass* NewClass = classBuilder->InitClass();
        if (NewClass) {
            LOG(LogRegistrar, Trace, "Registered class: {}", NewClass->GetName());
        } else {
            LOG(LogRegistrar, Error, "Failed to register class");
        }
    }
}

void Registrar::RegisteredClassBuilder(IClassBuilder* classBuilder)
{
    RegisteredClassBuilders.Add(classBuilder);
}
void Registrar::UnregisterClassBuilder(IClassBuilder* classBuilder)
{
    RegisteredClassBuilders.Remove(classBuilder);
}

/// Registers a type in the RTTI system.
/// @param type The type to register.
void Registrar::RegisterType(IType* type)
{
    RegisteredTypes.Add(type);
}

/// Unregisters a type from the RTTI system.
/// @param type The type to unregister.
void Registrar::UnregisterType(IType* type)
{
    RegisteredTypes.Remove(type);
}

/// Finds a type by name.
/// @param name The name of the type to find.
/// @return The type if found, nullptr otherwise.
[[nodiscard]] IType* Registrar::FindType(std::string_view name) const
{
    for (IType* type: RegisteredTypes) {
        printf("%s %s\n", type->GetName().data(), name.data());
        if (type->GetName().compare(name) == 0) {
            return type;
        }
    }
    return nullptr;
}

}    // namespace RTTI

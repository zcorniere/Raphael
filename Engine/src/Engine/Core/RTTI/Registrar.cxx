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
        if (type->GetName() == name) {
            return type;
        }
    }
    return nullptr;
}

}    // namespace RTTI

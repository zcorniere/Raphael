#include "Engine/Core/RTTI/RTTITypes.hxx"

#include "Engine/Core/RTTI/Registrar.hxx"

RTTI::FProperty::FProperty(FClass* OwnerClass, FName TypeName, FName Name, uint32 Offset, uint32 Alignment)
    : OwnerClass(OwnerClass),
      Type(Registrar::Get().FindType(TypeName)),
      Name(std::move(Name)),
      Offset(Offset),
      Alignment(Alignment)
{
    checkMsg(Type, "Type for {}::{} is not registered", OwnerClass->GetName(), Name);
}

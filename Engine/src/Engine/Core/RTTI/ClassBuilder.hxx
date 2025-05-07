#pragma once

#include "Engine/Core/RTTI/RTTITypes.hxx"
#include "Engine/Core/RTTI/Registrar.hxx"

DISABLE_WARNING("-Winvalid-offsetof")

namespace RTTI
{

class IClassBuilder
{
public:
    IClassBuilder()
    {
        RTTI::Registrar::Get().RegisteredClassBuilder(this);
    }
    virtual ~IClassBuilder() = default;

    virtual FClass* InitClass() = 0;
};

}    // namespace RTTI

#define RTTI_BEGIN_CLASS_DECLARATION(ClassName)                               \
    class ClassName##ClassBuilder : public RTTI::IClassBuilder                \
    {                                                                         \
        ::RTTI::FClass* RegisteredClass = nullptr;                            \
                                                                              \
    public:                                                                   \
        ClassName##ClassBuilder()                                             \
        {                                                                     \
            RegisteredClass = new ::RTTI::TTypedClass<ClassName>(#ClassName); \
            RTTI::Registrar::Get().RegisterType(RegisteredClass);             \
            ClassName::StaticClass = RegisteredClass;                         \
        }                                                                     \
                                                                              \
        ~ClassName##ClassBuilder()                                            \
        {                                                                     \
            RTTI::Registrar::Get().UnregisterType(RegisteredClass);           \
            delete RegisteredClass;                                           \
            RegisteredClass = nullptr;                                        \
            ClassName::StaticClass = nullptr;                                 \
        }                                                                     \
        virtual ::RTTI::FClass* InitClass() override                          \
        {                                                                     \
            using CurrentType = ClassName;

#define RTTI_END_CLASS_DECLARATION \
    return RegisteredClass;        \
    }                              \
    }                              \
    ;

#define PROPERTY(field)                                                                                       \
    {                                                                                                         \
        ::RTTI::FProperty Property(RegisteredClass, ::RTTI::TypeName<decltype(CurrentType::field)>(), #field, \
                                   offsetof(CurrentType, field), __alignof(CurrentType::field));              \
        RegisteredClass->AddProperty(std::move(Property));                                                    \
    }

#define DECLARE_RTTI(StructureName)           \
    friend class StructureName##ClassBuilder; \
                                              \
private:                                      \
    static ::RTTI::FClass* StaticClass;       \
                                              \
public:                                       \
    static ::RTTI::FClass* GetStaticClass()   \
    {                                         \
        return StaticClass;                   \
    }

#define IMPLEMENT_RTTI(StructureName)                                       \
    ::RTTI::FClass* TouchClass_##StructureName()                            \
    {                                                                       \
        return StructureName::GetStaticClass();                             \
    }                                                                       \
    static StructureName##ClassBuilder StructureName##ClassBuilderInstance; \
    RTTI::FClass* StructureName::StaticClass = nullptr;

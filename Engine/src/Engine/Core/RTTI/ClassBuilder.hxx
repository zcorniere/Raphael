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
        Registrar::Get().RegisteredClassBuilder(this);
    }
    virtual ~IClassBuilder() = default;

    virtual std::string_view GetName() const = 0;
    virtual FClass* InitClass() = 0;
};

}    // namespace RTTI

#define RTTI_BEGIN_CLASS_DECLARATION(ClassName)                                              \
    RTTI_DECLARE_NAME(ClassName);                                                            \
                                                                                             \
    class ClassName##ClassBuilder : public RTTI::IClassBuilder                               \
    {                                                                                        \
        ::RTTI::FClass* RegisteredClass = nullptr;                                           \
                                                                                             \
    public:                                                                                  \
        ClassName##ClassBuilder()                                                            \
        {                                                                                    \
            RegisteredClass = new ::RTTI::TTypedClass<ClassName>(::RTTI::FName(#ClassName)); \
            RTTI::Registrar::Get().RegisterType(RegisteredClass);                            \
            ClassName::StaticClass = RegisteredClass;                                        \
        }                                                                                    \
                                                                                             \
        ~ClassName##ClassBuilder()                                                           \
        {                                                                                    \
            RTTI::Registrar::Get().UnregisterType(RegisteredClass);                          \
            delete RegisteredClass;                                                          \
            RegisteredClass = nullptr;                                                       \
            ClassName::StaticClass = nullptr;                                                \
        }                                                                                    \
        virtual std::string_view GetName() const override                                    \
        {                                                                                    \
            return #ClassName;                                                               \
        }                                                                                    \
        virtual ::RTTI::FClass* InitClass() override                                         \
        {                                                                                    \
            using CurrentType = ClassName;

#define RTTI_END_CLASS_DECLARATION \
    return RegisteredClass;        \
    }                              \
    }                              \
    ;

#define RTTI_BEGIN_CLASS_DECLARATION_TEMPLATE(ClassName, TemplateParameter)                                           \
    template <typename TemplateParameter>                                                                             \
    class ClassName##ClassBuilder : public RTTI::IClassBuilder                                                        \
    {                                                                                                                 \
        ::RTTI::FClass* RegisteredClass = nullptr;                                                                    \
        const std::string ClassBuilderName;                                                                           \
                                                                                                                      \
    public:                                                                                                           \
        ClassName##ClassBuilder()                                                                                     \
            : ClassBuilderName(std::format(#ClassName "<{}>", ::RTTI::GetTypeName<TemplateParameter>()))              \
        {                                                                                                             \
            RegisteredClass = new ::RTTI::TTypedClass<ClassName<TemplateParameter>>(::RTTI::FName(ClassBuilderName)); \
            RTTI::Registrar::Get().RegisterType(RegisteredClass);                                                     \
            ClassName<TemplateParameter>::StaticClass = RegisteredClass;                                              \
        }                                                                                                             \
                                                                                                                      \
        ~ClassName##ClassBuilder()                                                                                    \
        {                                                                                                             \
            RTTI::Registrar::Get().UnregisterType(RegisteredClass);                                                   \
            delete RegisteredClass;                                                                                   \
            RegisteredClass = nullptr;                                                                                \
            ClassName<TemplateParameter>::StaticClass = nullptr;                                                      \
        }                                                                                                             \
        virtual std::string_view GetName() const override                                                             \
        {                                                                                                             \
            return ClassBuilderName;                                                                                  \
        }                                                                                                             \
        virtual ::RTTI::FClass* InitClass() override                                                                  \
        {                                                                                                             \
            using CurrentType = ClassName<TemplateParameter>;

#define __INTERNAL_PROPERTY(field, type)                                                          \
    ::RTTI::FProperty Property(RegisteredClass, ::RTTI::GetTypeName(type), ::RTTI::FName(#field), \
                               offsetof(CurrentType, field), __alignof(CurrentType::field));      \
    RegisteredClass->AddProperty(std::move(Property));

// Note: I need to do this horrible cast of adress 0 because of MSVC refusing the CurrentType::field patern. Because
// this is obviously better
#define PROPERTY(field)                                                  \
    {                                                                    \
        LOG(LogRegistrar, Trace, "Registering member: " #field);         \
        __INTERNAL_PROPERTY(field, static_cast<CurrentType*>(0)->field); \
    }

#define REF_PROPERTY(field)                                             \
    {                                                                   \
        LOG(LogRegistrar, Trace, "Registering Ref member: " #field);    \
        __INTERNAL_PROPERTY(field, static_cast<CurrentType*>(0)->field) \
    }

#define TYPED_PROPERTY(field, type)                                                                     \
    {                                                                                                   \
        LOG(LogRegistrar, Trace, "Registering member with forced type " #type " member: " #field);      \
        ::RTTI::FProperty Property(RegisteredClass, ::RTTI::GetTypeName<type>(), ::RTTI::FName(#field), \
                                   offsetof(CurrentType, field), __alignof(CurrentType::field));        \
        RegisteredClass->AddProperty(std::move(Property));                                              \
    }

// #TODO: check if parrent class is acually a FClass instead of just yolo casting
#define PARENT_CLASS(Parent)                                                                  \
    {                                                                                         \
        LOG(LogRegistrar, Trace, "Registering Parent class: " #Parent);                       \
        ::RTTI::IType* ParentClass = RTTI::Registrar::Get().FindType(::RTTI::FName(#Parent)); \
        check(ParentClass);                                                                   \
        RegisteredClass->AddParentClass(static_cast<::RTTI::FClass*>(ParentClass));           \
    }

#define DECLARE_RTTI(StructureName)           \
                                              \
    friend class StructureName##ClassBuilder; \
                                              \
private:                                      \
    static ::RTTI::FClass* StaticClass;       \
                                              \
public:                                       \
    static ::RTTI::FClass* GetStaticClass()   \
    {                                         \
        return StaticClass;                   \
    }                                         \
                                              \
private:

#define DECLARE_RTTI_CLASSBUILDER_TEMPLATE(StructureName, TemplateParameter) \
    template <typename TemplateParameter>                                    \
    class StructureName##ClassBuilder;

#define DECLARE_TEMPLATE_RTTI_NO_FRIEND(StructureName, TemplateParameter, ClassDeclaration) \
    ClassDeclaration:                                                                       \
    static ::RTTI::FClass* StaticClass;                                                     \
                                                                                            \
public:                                                                                     \
    static ::RTTI::FClass* GetStaticClass()                                                 \
    {                                                                                       \
        return StaticClass;                                                                 \
    }                                                                                       \
                                                                                            \
private:

#define DECLARE_TEMPLATE_RTTI(StructureName, TemplateParameter)  \
    friend class StructureName##ClassBuilder<TemplateParameter>; \
    DECLARE_TEMPLATE_RTTI_NO_FRIEND(StructureName, TemplateParameter, private)

#define IMPLEMENT_RTTI(StructureName)                                       \
    RTTI_DEFINE_NAME(StructureName)                                         \
    ::RTTI::FClass* TouchClass_##StructureName()                            \
    {                                                                       \
        return StructureName::GetStaticClass();                             \
    }                                                                       \
    static StructureName##ClassBuilder StructureName##ClassBuilderInstance; \
    RTTI::FClass* StructureName::StaticClass = nullptr;

/// WARNING: Cannot be used with "using" types, must be a full type name
#define IMPLEMENT_RTTI_TEMPLATE_STATIC_CLASS(StructureName, TemplateParameter) \
    template <typename TemplateParameter>                                      \
    RTTI::FClass* StructureName<TemplateParameter>::StaticClass = nullptr;

#define IMPLEMENT_RTTI_TEMPLATE(StructureName, TemplateParameter)                                   \
    RTTI_DEFINE_NAME_TEMPLATE(StructureName, TemplateParameter)                                     \
                                                                                                    \
    ::RTTI::FClass* MACRO_EXPENDER(TouchClass_, MACRO_EXPENDER(StructureName, TemplateParameter))() \
    {                                                                                               \
        return StructureName<TemplateParameter>::GetStaticClass();                                  \
    }                                                                                               \
    static StructureName##ClassBuilder<TemplateParameter> MACRO_EXPENDER(                           \
        StructureName, MACRO_EXPENDER(TemplateParameter, ClassBuilderInstance));

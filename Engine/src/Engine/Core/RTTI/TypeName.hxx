#pragma once

template <typename T>
class Ref;

template <typename T>
class WeakRef;

namespace RTTI
{
class FName;

template <typename Type>
struct TTypeName {
    static const FName& GetTypeName()
    {
        return Type::GetTypeName();
    }
};

template <class Type>
FORCEINLINE const FName& GetTypeName()
{
    return TTypeName<Type>::GetTypeName();
}

template <class Type>
FORCEINLINE const FName& GetTypeName(const Type&)
{
    return TTypeName<Type>::GetTypeName();
}

template <class Type>
FORCEINLINE const FName& GetTypeName(const Ref<Type>&)
{
    return TTypeName<Type>::GetTypeName();
}
template <class Type>
FORCEINLINE const FName& GetTypeName(const WeakRef<Type>&)
{
    return TTypeName<Type>::GetTypeName();
}

}    // namespace RTTI

#define RTTI_DEFINE_TYPENAME(Type, ...)                    \
    namespace RTTI                                         \
    {                                                      \
        template <>                                        \
        struct TTypeName<Type __VA_OPT__(<__VA_ARGS__>)> { \
            static const FName& GetTypeName()              \
            {                                              \
                return Names::name_##Type##__VA_ARGS__;    \
            }                                              \
        };                                                 \
    }

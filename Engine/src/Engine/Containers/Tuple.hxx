#pragma once

namespace __details::tuple
{

template <uint32 Index, typename TType>
struct TTupleElement {
    using Type = TType;

    TTupleElement(const TType& value): Value(value)
    {
    }
    TTupleElement(TType&& value): Value(std::move(value))
    {
    }
    TTupleElement()
    requires(std::is_default_constructible_v<Type>)
        : Value()
    {
    }
    TTupleElement(TTupleElement&&) = default;
    TTupleElement(const TTupleElement&) = default;
    TTupleElement& operator=(TTupleElement&&) = default;
    TTupleElement& operator=(const TTupleElement&) = default;

    Type Value;
};

template <uint32 Index, typename TType>
TType& get(TTupleElement<Index, TType>& element)
{
    return element.Value;
}

template <uint32 Index, typename TType>
const TType& get(const TTupleElement<Index, TType>& element)
{
    return element.Value;
}

template <typename Indices, typename... Types>
struct TTupleImpl;

template <uint32... Indices, typename... Types>
struct TTupleImpl<std::index_sequence<Indices...>, Types...> : TTupleElement<Indices, Types>... {
    TTupleImpl() = default;

    template <typename... Args>
    explicit TTupleImpl(Args&&... args): TTupleElement<Indices, Types>(std::forward<Args>(args))...
    {
    }
};

}    // namespace __details::tuple

template <typename... TTypes>
class TTuple : public __details::tuple::TTupleImpl<std::index_sequence_for<TTypes...>, TTypes...>
{

public:
    using __details::tuple::TTupleImpl<std::index_sequence_for<TTypes...>, TTypes...>::TTupleImpl;

    template <uint32 Index>
    requires(Index < sizeof...(TTypes))
    constexpr auto& Get()
    {
        return __details::tuple::get<Index>(*this);
    }

    template <uint32 Index>
    requires(Index < sizeof...(TTypes))
    constexpr const auto& Get() const
    {
        return __details::tuple::get<Index>(*this);
    }
};

template <typename Key, typename Value>
using TPair = TTuple<Key, Value>;

/// Structured bindings support
template <typename... ArgType>
struct std::tuple_size<TTuple<ArgType...>> : std::integral_constant<std::size_t, sizeof...(ArgType)> {
};

template <std::size_t Index, typename... ArgType>
struct std::tuple_element<Index, TTuple<ArgType...>> {

#ifdef COMPILER_CLANG    // Clang is the only one to support the __type_pack_element
    using type = __type_pack_element<Index, ArgType...>;
#else
private:
    template <typename DeducedType>
    static DeducedType Resolve(__details::tuple::TTupleElement<Index, DeducedType>*);
    static void Resolve(...);

public:
    using type = decltype(Resolve(std::declval<TTuple<ArgType...>*>()));
#endif
};

template <std::size_t Index, typename... ArgType>
constexpr decltype(auto) get(TTuple<ArgType...>&& tuple)
{
    return tuple.template Get<Index>();
}
/// End Structured bindings support

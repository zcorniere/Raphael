#pragma once

namespace __details::tuple
{

template <uint32 Index, typename TType>
struct TTupleElement {
    TTupleElement() = default;
    TTupleElement(const TType& Value): Value(Value)
    {
    }
    TTupleElement(TType&& Value): Value(std::move(Value))
    {
    }

    TType Value;
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
    auto Get()
    {
        return __details::tuple::get<Index>(*this);
    }

    template <uint32 Index>
    requires(Index < sizeof...(TTypes))
    auto Get() const
    {
        return __details::tuple::get<Index>(*this);
    }
};

template <typename Key, typename Value>
using TPair = TTuple<Key, Value>;

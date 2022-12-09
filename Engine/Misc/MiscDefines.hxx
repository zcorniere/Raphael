#pragma once

#define RAPHAEL_NONCOPYABLE(Name)           \
    Name(Name &&) = delete;                 \
    Name(const Name &) = delete;            \
    Name &operator=(const Name &) = delete; \
    Name &operator=(Name &&) = delete;

#define BIT(x) (1u << x)

#define CONSTEXPR_ELSE_ERROR(Type, Message) static_assert(AlwaysFalse<Type>, Message);

namespace Raphael
{

template <class...>
/// Used to error out in constexpr if
constexpr std::false_type AlwaysFalse{};

template <typename T>
/// Only accept hashable type
concept Hashable = requires(T a)
{
    {
        std::hash<T>{}(a)
        } -> std::convertible_to<std::size_t>;
};

}    // namespace Raphael
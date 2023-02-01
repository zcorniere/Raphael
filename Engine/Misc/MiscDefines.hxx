#pragma once

#define RPH_NONCOPYABLE(Name)               \
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
concept Hashable = requires(T a) {
                       {
                           std::hash<T>{}(a)
                           } -> std::convertible_to<std::size_t>;
                   };

}    // namespace Raphael

typedef std::uint8_t uint8;
typedef std::uint16_t uint16;
typedef std::uint32_t uint32;
typedef std::uint64_t uint64;

typedef std::int8_t int8;
typedef std::int16_t int16;
typedef std::int32_t int32;
typedef std::int64_t int64;

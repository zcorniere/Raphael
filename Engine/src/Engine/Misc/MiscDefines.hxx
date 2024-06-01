#pragma once

#include <magic_enum.hpp>
#include <magic_enum_utility.hpp>

#define __MACRO_EXPENDER_INTERNAL(X, Y) X##Y
#define MACRO_EXPENDER(X, Y) __MACRO_EXPENDER_INTERNAL(X, Y)

#define STR(x) #x

#define RPH_NONCOPYABLE(Name)              \
    Name(Name&&) = delete;                 \
    Name(const Name&) = delete;            \
    Name& operator=(const Name&) = delete; \
    Name& operator=(Name&&) = delete;

#define BIT(x) (1u << x)
#define NEXT_NEAREST(X, Multiple) ((((X) + (Multiple)-1) / (Multiple)) * (Multiple))

#define CONSTEXPR_ELSE_ERROR(Type, Message) static_assert(AlwaysFalse<Type>, Message);

#define DECLARE_PRINTABLE_TYPE(Type)   \
    template <>                        \
    struct std::formatter<Type, char>; \
    std::ostream& operator<<(std::ostream& os, const Type& m);
#define DEFINE_PRINTABLE_TYPE(Type, Format, ...)                     \
    template <>                                                      \
    struct std::formatter<Type, char> {                              \
        constexpr auto parse(format_parse_context& ctx)              \
        {                                                            \
            return begin(ctx);                                       \
        }                                                            \
                                                                     \
        template <class FormatContext>                               \
        auto format(const Type& Value, FormatContext& ctx) const     \
        {                                                            \
            auto&& out = ctx.out();                                  \
            format_to(out, Format, ##__VA_ARGS__);                   \
            return out;                                              \
        }                                                            \
    };                                                               \
    inline std::ostream& operator<<(std::ostream& os, const Type& m) \
    {                                                                \
        os << std::format("{}", m);                                  \
        return os;                                                   \
    }

template <typename...>
/// Used to error out in constexpr if
constexpr std::false_type AlwaysFalse{};

template <typename T>
/// Only accept hashable type
concept CHashable = requires(T a) {
    {
        std::hash<T>{}(a)
    } -> std::convertible_to<std::size_t>;
};

/// Is the template argument a function pointer ?
template <typename T>
concept CIsFunctionPointer = requires { std::is_pointer_v<T>&& std::is_function_v<typename std::remove_pointer<T>>; };

/// Is the types compatibles ?
template <typename TSource, typename TDest>
concept CCompatibleTypes =
    std::is_same<TDest, std::decay_t<TSource>>::Value || std::is_constructible<TDest, TSource>::value;

/// @cond
typedef std::uint8_t uint8;
typedef std::uint16_t uint16;
typedef std::uint32_t uint32;
typedef std::uint64_t uint64;

typedef std::int8_t int8;
typedef std::int16_t int16;
typedef std::int32_t int32;
typedef std::int64_t int64;
/// @endcond

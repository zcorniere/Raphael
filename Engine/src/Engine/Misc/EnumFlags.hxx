#pragma once

// Defines all bitwise operators for enum classes so it can be (mostly) used as a regular flags enum
#define ENUM_CLASS_FLAGS(Enum)                                                                      \
    inline Enum& operator|=(Enum& Lhs, Enum Rhs)                                                    \
    {                                                                                               \
        return Lhs = (Enum)((std::underlying_type_t<Enum>)Lhs | (std::underlying_type_t<Enum>)Rhs); \
    }                                                                                               \
    inline Enum& operator&=(Enum& Lhs, Enum Rhs)                                                    \
    {                                                                                               \
        return Lhs = (Enum)((std::underlying_type_t<Enum>)Lhs & (std::underlying_type_t<Enum>)Rhs); \
    }                                                                                               \
    inline Enum& operator^=(Enum& Lhs, Enum Rhs)                                                    \
    {                                                                                               \
        return Lhs = (Enum)((std::underlying_type_t<Enum>)Lhs ^ (std::underlying_type_t<Enum>)Rhs); \
    }                                                                                               \
    inline constexpr Enum operator|(Enum Lhs, Enum Rhs)                                             \
    {                                                                                               \
        return (Enum)((std::underlying_type_t<Enum>)Lhs | (std::underlying_type_t<Enum>)Rhs);       \
    }                                                                                               \
    inline constexpr Enum operator&(Enum Lhs, Enum Rhs)                                             \
    {                                                                                               \
        return (Enum)((std::underlying_type_t<Enum>)Lhs & (std::underlying_type_t<Enum>)Rhs);       \
    }                                                                                               \
    inline constexpr Enum operator^(Enum Lhs, Enum Rhs)                                             \
    {                                                                                               \
        return (Enum)((std::underlying_type_t<Enum>)Lhs ^ (std::underlying_type_t<Enum>)Rhs);       \
    }                                                                                               \
    inline constexpr bool operator!(Enum E)                                                         \
    {                                                                                               \
        return !(std::underlying_type_t<Enum>)E;                                                    \
    }                                                                                               \
    inline constexpr Enum operator~(Enum E)                                                         \
    {                                                                                               \
        return (Enum) ~(std::underlying_type_t<Enum>)E;                                             \
    }

template <typename Enum>
constexpr bool EnumHasAllFlags(Enum Flags, Enum Contains)
{
    return (((std::underlying_type_t<Enum>)Flags) & (std::underlying_type_t<Enum>)Contains) ==
           ((std::underlying_type_t<Enum>)Contains);
}

template <typename Enum>
constexpr bool EnumHasAnyFlags(Enum Flags, Enum Contains)
{
    return (((std::underlying_type_t<Enum>)Flags) & (std::underlying_type_t<Enum>)Contains) != 0;
}

template <typename Enum>
constexpr bool EnumHasAllFlags(Enum Flags, typename std::underlying_type_t<Enum> Contains)
{
    return (((std::underlying_type_t<Enum>)Flags) & Contains) == Contains;
}

template <typename Enum>
constexpr bool EnumHasAnyFlags(Enum Flags, typename std::underlying_type_t<Enum> Contains)
{
    return (((std::underlying_type_t<Enum>)Flags) & Contains) != 0;
}

template <typename Enum>
void EnumAddFlags(Enum& Flags, Enum FlagsToAdd)
{
    Flags |= FlagsToAdd;
}

template <typename Enum>
void EnumRemoveFlags(Enum& Flags, Enum FlagsToRemove)
{
    Flags &= ~FlagsToRemove;
}

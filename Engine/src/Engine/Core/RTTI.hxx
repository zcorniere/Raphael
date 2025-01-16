#pragma once

namespace RTTI
{

namespace details
{

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    // From
    // https://bitwizeshift.github.io/posts/2021/03/09/getting-an-unmangled-type-name-at-compile-time/#a-working-solution
    template <std::size_t... Idxs>
    consteval auto substring_as_array(std::string_view str, std::index_sequence<Idxs...>)
    {
        return std::array{str[Idxs]...};
    }

    template <typename T>
    consteval auto type_name_array()
    {
    #if defined(__clang__)
        constexpr auto prefix = std::string_view{"[T = "};
        constexpr auto suffix = std::string_view{"]"};
        constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
    #elif defined(__GNUC__)
        constexpr auto prefix = std::string_view{"with T = "};
        constexpr auto suffix = std::string_view{"]"};
        constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
    #elif defined(_MSC_VER)
        constexpr auto prefix = std::string_view{"type_name_array<class "};
        constexpr auto suffix = std::string_view{">(void)"};
        constexpr auto function = std::string_view{__FUNCSIG__};
    #else
        #error Unsupported compiler
    #endif

        constexpr auto start = function.find(prefix) + prefix.size();
        constexpr auto end = function.rfind(suffix);

        static_assert(start < end);

        constexpr auto name = function.substr(start, (end - start));
        return substring_as_array(name, std::make_index_sequence<name.size()>{});
    }

    template <typename T>
    struct type_name_holder {
        static inline constexpr auto value = type_name_array<T>();
    };
#endif

    namespace Hash
    {
        /// Calculates the 32bit FNV1a hash of a c-string literal.
        /// @param str String literal to be hashed
        /// @param n Length of the string.
        /// @return Calculated hash of the string
        static constexpr std::uint32_t FNV1a(const char* str, std::size_t n, std::uint32_t hash = UINT32_C(2166136261))
        {
            return n == 0 ? hash : FNV1a(str + 1, n - 1, (hash ^ str[0]) * UINT32_C(19777619));
        }

        /// Calculates the 32bit FNV1a hash of a std::string_view literal.
        /// note: Requires string_view to be a literal in order to be evaluated during compile time!
        /// @param str String literal to be hashed
        /// @return Calculated hash of the string
        static constexpr std::uint32_t FNV1a(std::string_view str)
        {
            return FNV1a(str.data(), str.size());
        }
    }    // namespace Hash
}    // namespace details

/// @brief Return the name of the type given as template arguments
template <typename T>
consteval std::string_view TypeName()
{
    constexpr auto& value = details::type_name_holder<T>::value;
    return std::string_view{value.data(), value.size()};
}

consteval const char* FunctionName(const std::source_location& location = std::source_location::current())
{
    return location.function_name();
}

inline std::string FilePosition(const std::source_location& location = std::source_location::current())
{
    return std::format("{:s}:{}", std::filesystem::path(location.file_name()).filename().string(), location.line());
}

struct FEnable;

/// Type identifier used to identify types in the RTTI system
using FTypeId = std::uint32_t;

template <typename TThis, typename... TParents>
concept IsRTTIValidEnabled =
    ((... && std::derived_from<TThis, TParents>)) && ((... && std::derived_from<FEnable, TParents>));

template <typename TThis>
concept IsRTTIEnabled = std::derived_from<TThis, FEnable>;

template <typename TThis, typename... TParents>
/// Static typeinfo structure for registering types and accessing their information.
struct TypeInfo {
    using T = std::remove_const_t<std::remove_reference_t<TThis>>;

    [[nodiscard]] static constexpr std::string_view Name() noexcept
    {
        return TypeName<T>();
    }

    /// Returns the type identifier of the type T.
    /// @returns Type identifier
    [[nodiscard]] static constexpr FTypeId Id() noexcept
    {
        return details::Hash::FNV1a(Name());
    }

    /// Checks whether the passed type is the same or a parent of the type.
    /// @tparam The type to compare the identifier with.
    /// @returns True in case a match was found.
    [[nodiscard]] static constexpr bool Is(FTypeId typeId) noexcept
    {
        return (Id() == typeId) || (... || (TParents::TypeInfo::Is(typeId)));
    }

    /// Walks the dependency hierarchy of the object in search of the type identified
    /// by the passed type id. In case found casts the passed pointer into the passed
    /// type. If no match can be found the function returns a nullptr.
    /// @tparam T The type of the most specialized type in the dependency hierarchy.
    /// @param typeId The identifier of the type to cast the object into.
    /// @returns Valid pointer to instance of requested type if the object is a
    /// direct descendance of the type identified by the passed type id. Otherwise
    /// the value returned is a nullptr.
    template <typename T>
    [[nodiscard]] static const void* DynamicCast(FTypeId typeId, const T* ptr) noexcept
    {
        // Check whether the current type matches the requested type.
        if (Id() == typeId) {
            // Cast the passed pointer in to the current type and stop
            // the recursion.
            return static_cast<const TThis*>(ptr);
        }

        const std::array<const void*, sizeof...(TParents)> ptrs = {TParents::TypeInfo::DynamicCast(typeId, ptr)...};

        // Check whether the traversal up the dependency hierarchy returned a pointer
        // that is not null.
        auto it = std::find_if(ptrs.begin(), ptrs.end(), [](const void* ptr) { return ptr != nullptr; });
        return (it != ptrs.end()) ? *it : nullptr;
    }
};

/// Parent type for types at the base of an open RTTI hierarchy
struct FEnable {
    virtual ~FEnable() = default;

    /// Returns the type identifier of the object.
    /// @returns Type identifier
    [[nodiscard]] virtual FTypeId TypeId() const noexcept = 0;

    /// Checks whether the object is a direct or derived instance of
    /// the type identified by the passed identifier.
    /// @tparam The identifier to compare with.
    /// @returns True in case a match was found.
    [[nodiscard]] virtual bool IsById(FTypeId typeId) const noexcept = 0;

    /// Checks whether the object is an instance of child instance of
    /// the passed type.
    /// @tparam The type to compare the identifier with.
    /// @returns True in case a match was found.
    template <typename T>
    [[nodiscard]] bool Is() const noexcept
    {
        return IsById(TypeInfo<T>::Id());
    }

    /// Dynamically cast the object to the passed type. Attempts to find the
    /// type by its identifier in the dependency hierarchy of the object.
    /// When found casts the top level `this` pointer to an instance the
    /// passed type using a static_cast. The pointer offset for types
    /// dependent on multiple inheritance is hence resolved at compile-time.
    /// In essence we are always up-casting from the most specialized type.
    /// @tparam T Pointer type to case the object into.
    /// @returns A valid pointer to an instance of the passed type. Nullptr
    /// incase the object instance is not a direct descendence of the passed
    /// type.
    template <typename T>
    [[nodiscard]] T* Cast() noexcept
    {
        return reinterpret_cast<T*>(const_cast<void*>(_cast(TypeInfo<T>::Id())));
    }

    template <typename T>
    [[nodiscard]] const T* Cast() const noexcept
    {
        return reinterpret_cast<T const*>(_cast(TypeInfo<T>::Id()));
    }

    bool operator==(const FEnable& other) const noexcept
    {
        return TypeId() == other.TypeId();
    }

protected:
    /// Used to invoke the dynamic_cast from the most specialized type in the
    /// dependency hierarchy by overloaded this function in each derivation of
    /// RTTI::Extends<>.
    /// @param typeId The identifier of the type to cast the object into.
    /// @returns Valid pointer to instance of requested type if the object is a
    /// direct descendance of the type identified by the passed type id. Otherwise
    /// the value returned is a nullptr.
    [[nodiscard]] virtual const void* _cast(FTypeId typeId) const noexcept = 0;
};

}    // namespace RTTI

#define __zz_RTTI_SUPER(T, ...) using Super = T;

/// Macro to be called in the body of each type declaration that is to be part of an
/// open hierarchy RTTI structure. The type itself or one or more parents of the type
/// need to have been derived from RTTI::Enable.
/// @param T The type it self.
/// @param Parents Variadic number of direct parent types of the type
#define RTTI_DECLARE_TYPEINFO(T, ...)                                                       \
public:                                                                                     \
    using TypeInfo = ::RTTI::TypeInfo<T, ##__VA_ARGS__>;                                    \
    [[nodiscard]] virtual RTTI::FTypeId TypeId() const noexcept override                    \
    {                                                                                       \
        return TypeInfo::Id();                                                              \
    }                                                                                       \
    [[nodiscard]] virtual bool IsById(::RTTI::FTypeId typeId) const noexcept override       \
    {                                                                                       \
        return TypeInfo::Is(typeId);                                                        \
    }                                                                                       \
    [[nodiscard]] std::string_view GetTypeName() const                                      \
    {                                                                                       \
        return TypeInfo::Name();                                                            \
    }                                                                                       \
                                                                                            \
protected:                                                                                  \
    [[nodiscard]] virtual const void* _cast(::RTTI::FTypeId typeId) const noexcept override \
    {                                                                                       \
        return TypeInfo::Is(typeId) ? TypeInfo::DynamicCast(typeId, this) : nullptr;        \
    }                                                                                       \
                                                                                            \
private:                                                                                    \
    __VA_OPT__(__zz_RTTI_SUPER(__VA_ARGS__))

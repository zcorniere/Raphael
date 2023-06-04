#pragma once

#include "Engine/Misc/MiscDefines.hxx"

#include <magic_enum.hpp>

/// Enumerates supported message dialog button types.
enum EBoxMessageType {
    Ok,
    YesNo,
};

/// Enumerates message dialog return types.
enum class EBoxReturnType {
    No,
    Yes,
    Ok,
    Cancel,
};

/// @brief Interface that represent a manually loaded shared library
class IExternalModule : public RObject
{
public:
    IExternalModule() = delete;
    /// @brief  Construct a new module, and load the library
    /// @param  Name The name of the shared library
    IExternalModule(std::string_view Name)
    {
        (void)Name;
    }
    virtual ~IExternalModule()
    {
    }

    /// @brief Find the requested function pointer
    /// @tparam T The Function pointer type to return
    /// @param SymbolName The name of the symbol to find
    /// @return The requested function pointer
    template <CIsFunctionPointer T>
    T GetSymbol(std::string_view SymbolName) const
    {
        return (T)GetSymbol_Internal(SymbolName);
    }

private:
    virtual void* GetSymbol_Internal(std::string_view SymbolName) const = 0;
};

DECLARE_LOGGER_CATEGORY(Core, LogPlatformMisc, Info);

/// @brief Miscellaneous platform agnostic function
class GenericMisc
{
public:
    /// @brief Display a simple message box
    /// @param Type The type of the message box (@see EBoxMessageType)
    /// @param Title The Title of the message box
    /// @param Caption The content of the message box
    /// @return The answer of the user (@see EBoxReturnType)
    static EBoxReturnType DisplayMessageBox(EBoxMessageType Type, const std::string_view Title,
                                            const std::string_view Caption)
    {
        LOG(LogPlatformMisc, Info, "{} Message Box: {:s} {:s}", magic_enum::enum_name(Type), Title, Caption);
        return EBoxReturnType::Ok;
    }

    /// @brief Platform independed function to load shared library
    /// If the same library is loaded multiple times, the return will be cached
    /// @param ModuleName The name of the module to load
    /// @return The loaded module
    static Ref<IExternalModule> LoadExternalModule(std::string_view ModuleName);

    /// @brief Platform agnostic way to look for a config file
    /// @return Return the platform standard path to look for the config
    static std::filesystem::path GetConfigPath();
};

#if defined(PLATFORM_WINDOWS)
    #include "Engine/Platforms/Windows/WindowsMisc.hxx"
#elif defined(PLATFORM_LINUX)
    #include "Engine/Platforms/Linux/LinuxMisc.hxx"
#else
static_assert(false, "Unsuported Platform !");
#endif

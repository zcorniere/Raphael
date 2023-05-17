#pragma once

#include "Engine/Misc/MiscDefines.hxx"

/**
 * Enumerates supported message dialog button types.
 */
enum EBoxMessageType {
    Ok,
    YesNo,
};

/**
 * Enumerates message dialog return types.
 */
enum class EBoxReturnType {
    No,
    Yes,
    Ok,
    Cancel,
};

class IExternalModule : public RObject
{
public:
    IExternalModule() = delete;
    IExternalModule(std::string_view) {}
    virtual ~IExternalModule() {}

    template <CIsFunctionPointer T>
    T GetSymbol(std::string_view SymbolName) const
    {
        return (T)GetSymbol_Internal(SymbolName);
    }

private:
    virtual void *GetSymbol_Internal(std::string_view SymbolName) const = 0;
};

DECLARE_LOGGER_CATEGORY(Core, LogPlatformMisc, Info);

class GenericMisc
{
public:
    static EBoxReturnType DisplayMessageBox(EBoxMessageType, const std::string_view Text, const std::string_view Caption)
    {
        LOG(LogPlatformMisc, Info, "Message Box: {:s} {:s}", Text, Caption);
        return EBoxReturnType::Ok;
    }

    static Ref<IExternalModule> LoadExternalModule(std::string_view ModuleName)
    {
        (void)ModuleName;
        return nullptr;
    }
};

#if defined(PLATFORM_WINDOWS)
    #include "Engine/Platforms/Windows/WindowsMisc.hxx"
#elif defined(PLATFORM_LINUX)
    #include "Engine/Platforms/Linux/LinuxMisc.hxx"
#else
static_assert(false, "Unsuported Platform !");
#endif

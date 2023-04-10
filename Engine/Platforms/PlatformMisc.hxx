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

DECLARE_LOGGER_CATEGORY(Core, LogPlatformMisc, Debug);

class GenericMisc
{
public:
    static EBoxReturnType MessageBox(EBoxMessageType, const std::string_view Text, const std::string_view Caption)
    {
        LOG(LogPlatformMisc, Info, "Message Box: {:s} {:s}", Text, Caption);
        return EBoxReturnType::Ok;
    }
};

#if defined(PLATFORM_WINDOWS)
    #include "Engine/Platforms/Windows/WindowsMisc.hxx"
#elif defined(PLATFORM_LINUX)
    #include "Engine/Platforms/Linux/LinuxMisc.hxx"
#else
static_assert(false, "Unsuported Platform !");
#endif

#pragma once

#include "Engine/Platforms/Platform.hxx"
// IWYU pragma: private, include "Platform.hxx"

#include <signal.h>
#include <sys/param.h>

#if !defined(PLATFORM_LINUX)

static_assert(false, "Unix Platform header included on a non Unix platform");

#endif

#define PLATFORM_BREAK() raise(SIGTRAP)

/// @brief Unix-specific functions
class FLinuxPlateform : public FGenericPlatform
{
private:
    /// Unix limit thread names to only 15 char + '\0'
    constexpr static unsigned UnixThreadNameLimit = 15;

public:
    /// @copydoc GenericPlatform::Initialize
    static void Initialize();

    /// @copydoc GenericPlatform::isDebuggerPresent
    static bool isDebuggerPresent();

    static std::filesystem::path GetExecutablePath();

    /// @copydoc GenericPlatform::setThreadName
    static void setThreadName(std::jthread& thread, const std::string& name);

    /// @copydoc GenericPlatform::getThreadName
    static std::string getThreadName(std::jthread& thread);
};

/// Alias to the current platform
using FPlatform = FLinuxPlateform;

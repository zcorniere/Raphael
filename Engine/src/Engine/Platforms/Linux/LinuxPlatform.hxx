#pragma once

#include <signal.h>

#include "Engine/Compilers/Compiler.hxx"
#include "Engine/Platforms/Platform.hxx"

#include <sys/param.h>

#if !defined(PLATFORM_LINUX)

static_assert(false, "Unix Platform header included on a non Unix platform");

#endif

#define PLATFORM_BREAK() raise(SIGTRAP)

/// @brief Unix-specific functions
class LinuxPlateform : public GenericPlatform
{
private:
    /// Unix limit thread names to only 15 char + '\0'
    constexpr static unsigned UnixThreadNameLimit = 15;

public:
    /// Is a debugger attached to the current process
    static bool isDebuggerPresent();

    static std::filesystem::path GetExecutablePath();

    ///
    /// @brief Set the Thread Name
    ///
    /// @param thread the handle of the thread to name
    /// @param name the new  name of the thread
    ///
    static void setThreadName(std::jthread& thread, const std::string& name);

    ///
    /// @brief Get the Thread Name
    ///
    /// @param thread the thread handle to get the name of
    /// @return std::string the name of the thread
    ///
    static std::string getThreadName(std::jthread& thread);
};

/// Alias to the current platform
using Platform = LinuxPlateform;

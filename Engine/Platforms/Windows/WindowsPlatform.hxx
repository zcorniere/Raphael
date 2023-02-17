#pragma once

#include "Engine/Compilers/Compiler.hxx"
#include "Engine/Platforms/Platform.hxx"

#if !defined(PLATFORM_WINDOWS)

static_assert(false, "Windows Platform header included on a non windows platform");

#endif

#include <intrin.h>

#define _CRT_SECURE_NO_WARNINGS
#define PLATFORM_BREAK() (__nop(), __debugbreak())

///
/// @brief Windows-specific function and commands
///
///
class WindowsPlatform : public GenericPlatform
{
public:
    /// Is a debugger attached to the current process
    static bool isDebuggerPresent();

    ///
    /// @brief Set the Thread Name
    ///
    /// @param thread the handle of the thread to name
    /// @param name the new  name of the thread
    ///
    static void setThreadName(std::jthread &thread, const std::string &name);

    ///
    /// @brief Get the Thread Name
    ///
    /// @param thread the thread handle to get the name of
    /// @return std::string the name of the thread
    ///
    static std::string getThreadName(std::jthread &thread);

    /// @brief Return a stacktrace of the current running process
    static StacktraceContent StackTrace(void *returnAddress);
};

/// Alias to the current platform
using Platform = WindowsPlatform;

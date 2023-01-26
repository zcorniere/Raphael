#pragma once

#include <string>
#include <thread>

#include "Engine/Misc/MiscDefines.hxx"

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS
#elif defined(__unix__) || defined(__unix)
    #define PLATFORM_LINUX
#endif

namespace Raphael::Platforms
{

constexpr auto MaxStacktraceDepth = 100;

struct StacktraceContent {
    std::uint32_t Depth;
    std::uint32_t CurrentDepth;
    int64 StackTrace[MaxStacktraceDepth];
    const std::uint32_t MaxDepth = MaxStacktraceDepth - 1;
};

///
/// @brief Unix-specific functions
///
class GenericPlatform
{
public:
    /// Is a debugger attached to the current process
    static bool isDebuggerPresent()
    {
        return false;
    }

    ///
    /// @brief Set the Thread Name
    ///
    /// @param thread the handle of the thread to name
    /// @param name the new  name of the thread
    ///
    static void setThreadName(std::jthread &thread, const std::string &name)
    {
    }

    ///
    /// @brief Get the Thread Name
    ///
    /// @param thread the thread handle to get the name of
    /// @return std::string the name of the thread
    ///
    static std::string getThreadName(std::jthread &thread)
    {
        return "";
    }

    /// @brief Return a stacktrace of the current running process
    static StacktraceContent StackTrace(void *returnAddress)
    {
        return {};
    }
};

}    // namespace Raphael::Platforms

#if defined(PLATFORM_WINDOWS)
    #include "Engine/Platforms/Windows/WindowsPlatform.hxx"
#elif defined(PLATFORM_LINUX)
    #include "Engine/Platforms/Linux/LinuxPlatform.hxx"
#else
static_assert(false, "Unsuported Platform !");
#endif

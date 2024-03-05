#pragma once

#include <string>
#include <thread>

#include "Engine/Misc/MiscDefines.hxx"

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS
#elif defined(__unix__) || defined(__unix)
    #define PLATFORM_LINUX
#endif

///
/// @brief Unix-specific functions
///
class GenericPlatform
{
public:
    /// Called before engine init, should the platform required it
    static void Initialize()
    {
    }

    /// Called after engine shutdown, should the platform required it
    static void Deinitialize()
    {
    }

    /// Is a debugger attached to the current process
    static bool isDebuggerPresent()
    {
        return false;
    }

    /// @brief Return the location of the executable
    /// @return Return the absolute path of the executable
    static std::filesystem::path GetExecutablePath()
    {
        return std::filesystem::current_path();
    }

    ///
    /// @brief Set the Thread Name
    ///
    /// @param thread the handle of the thread to name
    /// @param name the new  name of the thread
    ///
    static void setThreadName(std::jthread& thread, const std::string& name)
    {
        (void)thread;
        (void)name;
    }

    ///
    /// @brief Get the Thread Name
    ///
    /// @param thread the thread handle to get the name of
    /// @return std::string the name of the thread
    ///
    static std::string getThreadName(std::jthread& thread)
    {
        (void)thread;
        return "";
    }
};

#if defined(PLATFORM_WINDOWS)
    #include "Engine/Platforms/Windows/WindowsPlatform.hxx"    // IWYU pragma: export
#elif defined(PLATFORM_LINUX)
    #include "Engine/Platforms/Linux/LinuxPlatform.hxx"    // IWYU pragma: export
#else
static_assert(false, "Unsuported Platform !");
#endif

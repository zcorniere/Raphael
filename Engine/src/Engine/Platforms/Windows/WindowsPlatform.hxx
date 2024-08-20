#pragma once

#include "Engine/Platforms/Platform.hxx"
// IWYU pragma: private, include "Platform.hxx"

#include "Engine/Compilers/Compiler.hxx"

#if !defined(PLATFORM_WINDOWS)

static_assert(false, "Windows Platform header included on a non windows platform");

#endif

#include <intrin.h>

#define PLATFORM_BREAK() (__nop(), __debugbreak())

///
/// @brief Windows-specific function and commands
///
///
class WindowsPlatform : public GenericPlatform
{
public:
    /// @copydoc GenericPlatform::Initialize
    static void Initialize();

    /// @copydoc GenericPlatform::Deinitialize
    static void Deinitialize();

    /// @copydoc GenericPlatform::isDebuggerPresent
    static bool isDebuggerPresent();

    static std::filesystem::path GetExecutablePath();

    /// @copydoc GenericPlatform::setThreadName
    static void setThreadName(std::jthread& thread, const std::string& name);

    /// @copydoc GenericPlatform::getThreadName
    static std::string getThreadName(std::jthread& thread);
};

/// Alias to the current platform
using Platform = WindowsPlatform;

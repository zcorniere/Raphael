#pragma once

#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS
#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
    #define PLATFORM_UNIX
#endif

#if defined(PLATFORM_WINDOWS)
    #include "Engine/Platforms/Windows/WindowsPlatform.hxx"
#elif defined(PLATFORM_UNIX)
    #include "Engine/Platforms/Unix/UnixPlatform.hxx"
#else
static_assert(false, "Unsuported Platform !");
#endif

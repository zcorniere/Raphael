#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS
#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
    #define PLATFORM_UNIX
#endif

namespace Raphael::Platforms
{

constexpr auto MaxStacktraceDepth = 100;

struct StacktraceContent {
    std::uint32_t Depth;
    std::uint32_t CurrentDepth;
    std::uint64_t StackTrace[MaxStacktraceDepth];
    const std::uint32_t MaxDepth = MaxStacktraceDepth - 1;
};

}    // namespace Raphael::Platforms

#if defined(PLATFORM_WINDOWS)
    #include "Engine/Platforms/Windows/WindowsPlatform.hxx"
#elif defined(PLATFORM_UNIX)
    #include "Engine/Platforms/Unix/UnixPlatform.hxx"
#else
static_assert(false, "Unsuported Platform !");
#endif

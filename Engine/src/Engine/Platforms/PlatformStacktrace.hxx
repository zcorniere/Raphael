#pragma once

#include "Engine/Misc/MiscDefines.hxx"

/// @brief Holds information about the symbol located at ProgramCounter.
///
/// This struct is supposed to only be used during the stacktrace generation, and should not be cached !
struct DetailedSymbolInfo {
    static constexpr auto MaxNameLength = 1024;

    /// Name of the module (executable name or shared library name)
    char ModuleName[MaxNameLength] = {'\0'};
    /// Name of the function (mangled) @see Compiler::Demangle
    char FunctionName[MaxNameLength] = {'\0'};
    int64 ProgramCounter;

     char Filename[MaxNameLength] = {'\0'};
     int32 LineNumber;
};

/// @brief Hold the raw information about a stacktrace
struct StacktraceContent {
    static constexpr auto MaxDepth = 99;

    /// Max Depth of the of stack trace
    std::uint32_t Depth;
    /// @brief The first index of the stack trace that is interesting
    ///
    /// Skip the stacktrace collection function for example
    std::uint32_t CurrentDepth;
    /// @brief Holds the stacktraces function pointers
    int64 StackTrace[MaxDepth];
    /// @brief Absolute max depth
};

/// Generic stacktrace collection
class GenericStacktrace
{
public:
    /// @brief Return a stacktrace of the current running process
    static StacktraceContent GetStackTraceFromReturnAddress(void* returnAddress)
    {
        (void)returnAddress;
        return {};
    }

    /// @brief Try to gather information on the symbol located at ProgramCounter
    /// @return false if no information was found, true otherwise (even if the information are incomplete)
    static bool TryFillDetailedSymbolInfo(int64 ProgramCounter, DetailedSymbolInfo& detailed_info)
    {
        (void)ProgramCounter;
        (void)detailed_info;
        return false;
    }
};

#if defined(PLATFORM_WINDOWS)
    #include "Engine/Platforms/Windows/WindowsStacktrace.hxx"    // IWYU pragma: export
#elif defined(PLATFORM_LINUX)
    #include "Engine/Platforms/Linux/LinuxStacktrace.hxx"    // IWYU pragma: export
#else
static_assert(false, "Unsuported Platform !");
#endif

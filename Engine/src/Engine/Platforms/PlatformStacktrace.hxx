#pragma once

#include "Engine/Misc/MiscDefines.hxx"

struct DetailedSymbolInfo final {
    static constexpr auto MaxNameLength = 1024;

    char ModuleName[MaxNameLength] = {'\0'};
    char FunctionName[MaxNameLength] = {'\0'};
    // TODO: find out how to recover source file information
    // char Filename[MaxNameLength] = {'\0'};
    // int32 LineNumber;
    int64 ProgramCounter;
};

struct StacktraceContent {
    static constexpr auto MaxStacktraceDepth = 100;

    std::uint32_t Depth;
    std::uint32_t CurrentDepth;
    int64 StackTrace[MaxStacktraceDepth];
    const std::uint32_t MaxDepth = MaxStacktraceDepth - 1;
};

class GenericStacktrace
{
public:
    /// @brief Return a stacktrace of the current running process
    static StacktraceContent GetStackTraceFromReturnAddress(void* returnAddress)
    {
        (void)returnAddress;
        return {};
    }

    static bool TryFillDetailedSymbolInfo(int64 ProgramCounter, DetailedSymbolInfo& detailed_info)
    {
        (void)ProgramCounter;
        (void)detailed_info;
        return false;
    }
};

#if defined(PLATFORM_WINDOWS)
    #include "Engine/Platforms/Windows/WindowsStacktrace.hxx"
#elif defined(PLATFORM_LINUX)
    #include "Engine/Platforms/Linux/LinuxStacktrace.hxx"
#else
static_assert(false, "Unsuported Platform !");
#endif

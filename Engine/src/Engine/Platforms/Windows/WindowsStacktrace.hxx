#pragma once

#include "Engine/Platforms/PlatformStacktrace.hxx"

class WindowsStacktrace : public GenericStacktrace
{
public:
    /// @brief Return a stacktrace of the current running process
    static StacktraceContent GetStackTraceFromReturnAddress(void* returnAddress);

    static bool TryFillDetailedSymbolInfo(int64 ProgramCounter, DetailedSymbolInfo& detailed_info);
};
using PlatformStacktrace = WindowsStacktrace;

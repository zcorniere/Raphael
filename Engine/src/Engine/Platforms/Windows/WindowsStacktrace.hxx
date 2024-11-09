#pragma once

#include "Engine/Platforms/PlatformStacktrace.hxx"
// IWYU pragma: private, include "PlatformStacktrace.hxx"

class FWindowsStacktrace : public FGenericStacktrace
{
public:
    /// @brief Return a stacktrace of the current running process
    static StacktraceContent GetStackTraceFromReturnAddress(void* returnAddress);

    static bool TryFillDetailedSymbolInfo(int64 ProgramCounter, DetailedSymbolInfo& detailed_info);
};
using FPlatformStacktrace = FWindowsStacktrace;

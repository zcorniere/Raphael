#include "Engine/Platforms/Windows/WindowsStacktrace.hxx"

StacktraceContent WindowsStacktrace::GetStackTraceFromReturnAddress(void *returnAddress)
{
    return {};
}

bool WindowsStacktrace::TryFillDetailedSymbolInfo(int64 ProgramCounter, DetailedSymbolInfo &detailed_info)
{
    return false;
}


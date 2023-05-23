#include "Engine/Platforms/Windows/WindowsStacktrace.hxx"

StacktraceContent WindowsStacktrace::GetStackTraceFromReturnAddress(void* returnAddress)
{
    (void)returnAddress;
    return {};
}

bool WindowsStacktrace::TryFillDetailedSymbolInfo(int64 ProgramCounter, DetailedSymbolInfo& detailed_info)
{
    (void)ProgramCounter;
    (void)detailed_info;
    return false;
}

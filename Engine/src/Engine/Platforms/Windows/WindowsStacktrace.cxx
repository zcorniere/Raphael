#include "Engine/Platforms/Windows/WindowsStacktrace.hxx"

#include <windows.h>
#include <dbghelp.h>


StacktraceContent WindowsStacktrace::GetStackTraceFromReturnAddress(void* returnAddress)
{
    StacktraceContent trace;
    std::memset(&trace, 0, sizeof(StacktraceContent));
    trace.Depth = CaptureStackBackTrace(0, trace.MaxDepth, reinterpret_cast<void**>(trace.StackTrace), 0);
    trace.CurrentDepth = 0;

    if (returnAddress != nullptr) {
        for (std::uint32_t i = 0; i < trace.Depth; ++i) {
            if (trace.StackTrace[i] != int64(returnAddress)) {
                continue;
            }
            trace.CurrentDepth = i;
            break;
        }
    }
    return trace;
}

bool WindowsStacktrace::TryFillDetailedSymbolInfo(int64 ProgramCounter, DetailedSymbolInfo& detailed_info)
{
    DWORD64  dwDisplacement = 0;
    DWORD64  dwAddress = ProgramCounter;
    detailed_info.ProgramCounter = ProgramCounter;

    char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
    PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;

    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    pSymbol->MaxNameLen = MAX_SYM_NAME;
    if (!SymFromAddr(GetCurrentProcess(), dwAddress, &dwDisplacement, pSymbol))
    {
        DWORD error = GetLastError();
        printf("SymFromAddr returned error : %d\n", error);
        return false;
    }

    std::strncpy(detailed_info.FunctionName, pSymbol->Name, detailed_info.MaxNameLength);

    return true;
}

#include "Engine/Platforms/Windows/WindowsStacktrace.hxx"

#include "Engine/Platforms/Windows/WindowsPlatform.hxx"

#include <windows.h>

#include <dbghelp.h>

StacktraceContent FWindowsStacktrace::GetStackTraceFromReturnAddress(void* returnAddress)
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

bool FWindowsStacktrace::TryFillDetailedSymbolInfo(int64 ProgramCounter, DetailedSymbolInfo& detailed_info)
{
    DWORD64 dwAddress = ProgramCounter;
    detailed_info.ProgramCounter = ProgramCounter;

    // Function Name
    {
        DWORD64 dwDisplacement = 0;

        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;

        pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        pSymbol->MaxNameLen = MAX_SYM_NAME;
        if (!SymFromAddr(FWindowsPlatform::GetDebugSymbolHandle(), dwAddress, &dwDisplacement, pSymbol)) {
            DWORD error = GetLastError();
            printf("SymFromAddr returned error : %d\n", error);
            return false;
        }
        std::strncpy(detailed_info.FunctionName, pSymbol->Name, detailed_info.MaxNameLength);
    }

    {
        IMAGEHLP_MODULE ModuleInfo;
        ModuleInfo.SizeOfStruct = sizeof(ModuleInfo);

        if (SymGetModuleInfo64(FWindowsPlatform::GetDebugSymbolHandle(), dwAddress, &ModuleInfo)) {
            const char* const ModulePath = ModuleInfo.ImageName;
            const char* ModuleName = std::strrchr(ModulePath, '\\');
            if (ModuleName) {
                ModuleName += 1;
            } else {
                ModuleName = ModulePath;
            }
            std::strncpy(detailed_info.ModuleName, ModuleName, DetailedSymbolInfo::MaxNameLength);
        }
    }

    {
        DWORD dwDisplacement = 0;
        IMAGEHLP_LINE64 line;
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

        if (SymGetLineFromAddr64(FWindowsPlatform::GetDebugSymbolHandle(), dwAddress, &dwDisplacement, &line)) {
            const char* const cFilePath = line.FileName;
            const char* cFilenamePath = std::strrchr(cFilePath, '\\');
            if (cFilenamePath) {
                cFilenamePath += 1;
            } else {
                cFilenamePath = cFilePath;
            }
            std::strncpy(detailed_info.Filename, cFilenamePath, DetailedSymbolInfo::MaxNameLength);
            detailed_info.LineNumber = line.LineNumber;
        }
    }

    return true;
}

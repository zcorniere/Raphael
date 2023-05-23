#include "Engine/Platforms/Linux/LinuxStacktrace.hxx"

#include <dlfcn.h>
#include <execinfo.h>
#include <fcntl.h>

StacktraceContent LinuxStacktrace::GetStackTraceFromReturnAddress(void* returnAddress)
{
    StacktraceContent trace;
    trace.Depth = backtrace(reinterpret_cast<void**>(trace.StackTrace), trace.MaxDepth);

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

bool LinuxStacktrace::TryFillDetailedSymbolInfo(int64 ProgramCounter, DetailedSymbolInfo& detailed_info)
{
    Dl_info info;
    bool ret = dladdr(reinterpret_cast<void*>(ProgramCounter), &info);

    if (ret) {
        detailed_info.ProgramCounter = ProgramCounter;

        const char* ModulePath = info.dli_fname;
        const char* ModuleName = std::strrchr(ModulePath, '/');
        if (ModuleName) {
            ModuleName += 1;
        } else {
            ModuleName = ModulePath;
        }
        std::strcpy(detailed_info.ModuleName, ModuleName);
        if (info.dli_sname != nullptr) {
            std::strcpy(detailed_info.FunctionName, info.dli_sname);
        }
    }
    return ret;
}

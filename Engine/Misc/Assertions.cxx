#include "Engine/Misc/Assertions.hxx"
#include "Engine/Misc/UniquePtr.hxx"

#include <dlfcn.h>

DECLARE_LOGGER_CATEGORY(Core, LogAssert, Trace)

namespace Raphael::Assertions
{

struct DetailedSymbolInfo final {
    static constexpr unsigned MaxNameLength = 1024;

    char ModuleName[MaxNameLength] = {'\0'};
    char FunctionName[MaxNameLength] = {'\0'};
    // TODO: find out how to recover source file information
    // char Filename[MaxNameLength] = {'\0'};
    // std::int32_t LineNumber;
    std::uint64_t ProgramCounter;
};

bool TryFillDetailedSymbolInfo(std::uint64_t ProgramCounter, DetailedSymbolInfo &detailed_info)
{
    Dl_info info;
    bool ret = dladdr(reinterpret_cast<void *>(ProgramCounter), &info);

    if (ret) {
        detailed_info.ProgramCounter = ProgramCounter;

        const char *ModulePath = info.dli_fname;
        const char *ModuleName = std::strrchr(ModulePath, '/');
        if (ModuleName) {
            ModuleName += 1;
        } else {
            ModuleName = ModulePath;
        }
        std::strcpy(detailed_info.ModuleName, ModuleName);
        if (info.dli_sname != nullptr) { std::strcpy(detailed_info.FunctionName, info.dli_sname); }
    }
    return ret;
}

void CollectAndPrintStackTrace(void *ReturnAddress)
{
    Platforms::StacktraceContent trace = Platform::StackTrace(ReturnAddress);

    LOG(LogAssert, Trace, "StackTrace :");
    for (std::uint32_t CurrentDepth = trace.CurrentDepth; CurrentDepth < trace.Depth; CurrentDepth++) {
        DetailedSymbolInfo detailed_info;
        TryFillDetailedSymbolInfo(trace.StackTrace[CurrentDepth], detailed_info);

        std::string demangled = Compiler::Demangle(detailed_info.FunctionName);
        void *ProgramCounter = reinterpret_cast<void *>(detailed_info.ProgramCounter);
        LOG(LogAssert, Trace, "{} {} [{}]", ProgramCounter,
            (detailed_info.FunctionName[0] == '\0') ? ("UnknownFunction") : (demangled), detailed_info.ModuleName);
    }
}

}    // namespace Raphael::Assertions

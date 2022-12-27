#include "Engine/Misc/Assertions.hxx"
#include "Engine/Misc/UniquePtr.hxx"

#include <dlfcn.h>

DECLARE_LOGGER_CATEGORY(Core, LogAssert, Trace)

namespace Raphael::Assertions
{

struct SymbolInfo {
    void *addr;
    std::string_view symbolName;
    std::string_view moduleName;
};

void CollectAndPrintStackTrace(void *ReturnAddress)
{
    Platforms::StacktraceContent trace = Platform::StackTrace(ReturnAddress);
    std::vector<SymbolInfo> dladdr_names(trace.Depth - trace.CurrentDepth);

    for (std::uint32_t CurrentDepth = trace.CurrentDepth; CurrentDepth < trace.Depth; CurrentDepth++) {
        Dl_info info;
        bool ret = dladdr(reinterpret_cast<void *>(trace.StackTrace[CurrentDepth]), &info);
        dladdr_names[CurrentDepth - trace.CurrentDepth] = {
            .addr = reinterpret_cast<void *>(trace.StackTrace[CurrentDepth]),
        };
        if (LIKELY(ret)) {
            if (info.dli_sname)
                dladdr_names[CurrentDepth - trace.CurrentDepth].symbolName =
                    std::string_view(info.dli_sname, std::strlen(info.dli_sname));
            if (info.dli_fname)
                dladdr_names[CurrentDepth - trace.CurrentDepth].moduleName =
                    std::string_view(info.dli_fname, std::strlen(info.dli_fname));
        } else {
            LOG(LogAssert, Fatal, "dladdr failed to read symbol at {} : {}",
                reinterpret_cast<void *>(trace.StackTrace[CurrentDepth]), dlerror());
        }
    }

    LOG(LogAssert, Trace, "StackTrace :");
    for (const auto &[SymbolAddr, SymbolName, ModuleName]: dladdr_names) {
        std::string demangled = Compiler::Demangle(SymbolName);
        LOG(LogAssert, Trace, "{} {} [{}]", SymbolAddr, demangled.empty() ? "UnknownFunction" : demangled, ModuleName);
    }
}

}    // namespace Raphael::Assertions

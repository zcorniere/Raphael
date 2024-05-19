#include "Engine/Platforms/PlatformStacktrace.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogAssert, Trace)

namespace Raphael
{

void CollectAndPrintStackTrace(void* ReturnAddress)
{
    static bool bIsAlreadyHandlerAssertions = false;

    if (bIsAlreadyHandlerAssertions)
        return;
    bIsAlreadyHandlerAssertions = true;

    StacktraceContent trace = PlatformStacktrace::GetStackTraceFromReturnAddress(ReturnAddress);

    LOG(LogAssert, Trace, "StackTrace :");
    for (std::uint32_t CurrentDepth = trace.CurrentDepth; CurrentDepth < trace.Depth; CurrentDepth++) {
        DetailedSymbolInfo detailed_info;
        PlatformStacktrace::TryFillDetailedSymbolInfo(trace.StackTrace[CurrentDepth], detailed_info);

        std::string demangled = Compiler::Demangle(detailed_info.FunctionName);
        void* ProgramCounter = reinterpret_cast<void*>(detailed_info.ProgramCounter);
        LOG(LogAssert, Trace, "{} {} [{}]", ProgramCounter,
            (detailed_info.FunctionName[0] == '\0') ? ("UnknownFunction") : (demangled), detailed_info.ModuleName);
    }
    bIsAlreadyHandlerAssertions = false;
}

}    // namespace Raphael

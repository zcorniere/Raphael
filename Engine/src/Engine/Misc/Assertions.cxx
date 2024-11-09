#include "Engine/Platforms/PlatformStacktrace.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogAssert, Trace)

bool Raphael::Debug::HandleCheckFailure(const std::string& Message, bool bShouldAbort)
{
    if constexpr (ShouldCheckPrintStackTrace()) {
        CollectAndPrintStackTrace(Compiler::ReturnAddress());
    }

    fprintf(stderr, "%s\n", Message.c_str());
    fflush(stderr);

    if (bShouldAbort) {
        if (FPlatform::isDebuggerPresent()) {
            PLATFORM_BREAK();
        } else {
            std::abort();
        }
    }
    return FPlatform::isDebuggerPresent();
}

void Raphael::Debug::CollectAndPrintStackTrace(void* ReturnAddress)
{
    static bool bIsAlreadyHandlerAssertions = false;

    if (bIsAlreadyHandlerAssertions)
        return;
    bIsAlreadyHandlerAssertions = true;

    StacktraceContent trace = FPlatformStacktrace::GetStackTraceFromReturnAddress(ReturnAddress);

    LOG(LogAssert, Trace, "StackTrace :");
    for (std::uint32_t CurrentDepth = trace.CurrentDepth; CurrentDepth < trace.Depth; CurrentDepth++) {
        DetailedSymbolInfo detailed_info;
        std::memset(&detailed_info, 0, sizeof(detailed_info));
        std::strcpy(detailed_info.Filename, "Unknown file");
        FPlatformStacktrace::TryFillDetailedSymbolInfo(trace.StackTrace[CurrentDepth], detailed_info);

        std::string demangled = Compiler::Demangle(detailed_info.FunctionName);
        void* ProgramCounter = reinterpret_cast<void*>(detailed_info.ProgramCounter);
        LOG(LogAssert, Trace, "{} {} [{}] ({}:{})", ProgramCounter,
            (detailed_info.FunctionName[0] == '\0') ? ("UnknownFunction") : (demangled), detailed_info.ModuleName,
            detailed_info.Filename, detailed_info.LineNumber);
    }
    bIsAlreadyHandlerAssertions = false;
}

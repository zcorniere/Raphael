#include "Engine/Compilers/ClangCompiler.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogClangCompiler, Info);

std::string ClangCompiler::Demangle(const std::string_view& name)
{
    if (name.empty())
        return "";

    int status = 0;
    std::size_t xsize = 0;
    char* const demangled = abi::__cxa_demangle(name.data(), NULL, &xsize, &status);

    if (status != 0)
    {
        // Ignore Not a valid mangled name (-2) error
        LOG(LogClangCompiler, Trace, "Failed to demangle \"{}\"({})", name, status);
        return std::string(name);
    }
    std::string DemangledString(demangled);
    free(const_cast<char*>(demangled));
    return DemangledString;
}

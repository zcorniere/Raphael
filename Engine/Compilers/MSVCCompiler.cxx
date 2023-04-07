#include "Engine/Compilers/MSVCCompiler.hxx"

#include <dbghelp.h>

std::string MSVCCompiler::Demangle(const std::string_view &name)
{
    char undecorated_name[1024];
    UnDecorateSymbolName(name.data(), undecorated_name, sizeof(undecorated_name) / sizeof(*undecorated_name), UNDNAME_COMPLETE);
    return std::string(undecorated_name);
}

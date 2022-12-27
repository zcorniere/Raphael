#include "Engine/Compilers/MSVCCompiler.hxx"

namespace Raphael::Compilers
{

static std::string MSVCCompiler::Demangle(const std::string_view &name)
{
    checkMsg(false, "Not implemented");
    return std::string(name);
}

}    // namespace Raphael::Compilers

#pragma once

#include <string>

#if defined(__clang__)
    #define COMPILER_CLANG
#elif defined(__GNUC__)
    #define COMPILER_GNU
#elif defined(_MSC_VER)
    #define COMPILER_MSVC
#endif

///
/// @brief Wrapper arround generic intrinsics function
///
class GenericCompiler
{
public:
    /// Return the address of the current function,
    static void* ReturnAddress()
    {
        return nullptr;
    }

    /// Mark a branch as unreachable
    [[noreturn]] static void Unreachable()
    {
        std::abort();
    }

    /// Try to demangle the function name given as argument
    static std::string Demangle(const std::string_view& name)
    {
        return std::string(name);
    }
};

#if defined(COMPILER_CLANG)
    #include "Engine/Compilers/ClangCompiler.hxx"
#elif defined(COMPILER_GNU)
    #include "Engine/Compilers/GNUCompiler.hxx"
#elif defined(COMPILER_MSVC)
    #include "Engine/Compilers/MSVCCompiler.hxx"
#else
    #error "Unsuported Compiler !"
#endif

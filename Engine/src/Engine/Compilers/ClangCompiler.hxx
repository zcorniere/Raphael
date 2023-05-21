#pragma once

#include "Engine/Compilers/Compiler.hxx"
#include "Engine/Misc/UniquePtr.hxx"
#include "Engine/Platforms/Platform.hxx"

#include <cxxabi.h>

#if !defined(COMPILER_CLANG)

static_assert(false, "Clang Compiler header included without compiling with clang");

#endif

///
/// @brief Wrapper arround clang intrinsics function
///
class ClangCompiler : public GenericCompiler
{
public:
    /// Return the address of the current function,
    FORCEINLINE static void *ReturnAddress()
    {
        return __builtin_return_address(0);
    }

    /// Mark a branch as unreachable
    [[noreturn]] FORCEINLINE static void Unreachable()
    {
        __builtin_unreachable();
    }

    static std::string Demangle(const std::string_view &name);
};

/// Alias of the correct compiler currently used
using Compiler = ClangCompiler;
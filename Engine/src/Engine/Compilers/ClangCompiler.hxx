#pragma once

#include "Engine/Compilers/Compiler.hxx"

#include <cxxabi.h>

#if !defined(COMPILER_CLANG)

static_assert(false, "Clang Compiler header included without compiling with clang");

#endif

#define FORCEINLINE inline __attribute__((always_inline)) /* Force code to be inline */
#define FORCENOINLINE __attribute__((noinline))           /* Force code to NOT be inline */

#define ASSUME(...) __builtin_assume((__VA_ARGS__)) /* Assume code will ALWAYS be true */

///
/// @brief Wrapper arround clang intrinsics function
///
class ClangCompiler : public GenericCompiler
{
public:
    /// Return the address of the current function,
    FORCEINLINE static void* ReturnAddress()
    {
        return __builtin_return_address(0);
    }

    /// Mark a branch as unreachable
    [[noreturn]] FORCEINLINE static void Unreachable()
    {
        __builtin_unreachable();
    }

    static std::string Demangle(const std::string_view& name);
};

/// Alias of the correct compiler currently used
using Compiler = ClangCompiler;

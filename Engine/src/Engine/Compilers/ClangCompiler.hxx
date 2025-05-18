#pragma once

#include "Engine/Compilers/Compiler.hxx"
// IWYU pragma: private, include "Compiler.hxx"

#include <cxxabi.h>

#if !defined(COMPILER_CLANG)

static_assert(false, "Clang Compiler header included without compiling with clang");

#endif

#define FORCEINLINE inline __attribute__((always_inline)) /* Force code to be inline */
#define FORCENOINLINE __attribute__((noinline))           /* Force code to NOT be inline */

#define ASSUME(...) __builtin_assume((__VA_ARGS__)) /* Assume code will ALWAYS be true */

#define DISABLE_WARNING_PUSH _Pragma("GCC diagnostic push")
#define DISABLE_WARNING_POP _Pragma("GCC diagnostic pop")

#define __PRAGMA_INTERNAL(X) _Pragma(#X)
#define DISABLE_WARNING(warning) __PRAGMA_INTERNAL(clang diagnostic ignored warning)

///
/// @brief Wrapper around clang intrinsics function
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

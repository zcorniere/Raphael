#pragma once

#include "Engine/Compilers/Compiler.hxx"
// IWYU pragma: private, include "Compiler.hxx"

#include "Engine/Platforms/Platform.hxx"

#include <cxxabi.h>

#if !defined(COMPILER_GNU)

static_assert(false, "GNU Compiler header included without compiling with GNU");

#endif

#define FORCEINLINE inline __attribute__((always_inline)) /* Force code to be inline */
#define FORCENOINLINE __attribute__((noinline))           /* Force code to NOT be inline */

#define ASSUME(...)                  \
    {                                \
        if (!(__VA_ARGS__))          \
            __builtin_unreachable(); \
    }

#define RESTRICT __restrict__

///
/// @brief Wrapper around GCC intrinsics functions
///
class GNUCompiler : public GenericCompiler
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
using Compiler = GNUCompiler;

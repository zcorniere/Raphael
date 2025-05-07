#pragma once

#include "Engine/Compilers/Compiler.hxx"
// IWYU pragma: private, include "Compiler.hxx"

#include "Engine/Platforms/Platform.hxx"

#if !defined(COMPILER_MSVC)

static_assert(false, "MSVC Compiler header included without compiling with MSVC");

#endif

#include <intrin.h>

#ifndef FORCEINLINE
    #define FORCEINLINE inline __forceinline /* Force code to be inline */
#endif

#ifndef FORCENOINLINE
    #define FORCENOINLINE __declspec(noinline) /* Force code NOT to be inline */
#endif

#define ASSUME(...) __assume((__VA_ARGS__)) /* Assume code will ALWAYS be true */

#define DISABLE_WARNING_PUSH __pragma(warning(push))
#define DISABLE_WARNING_POP __pragma(warning(pop))
#define DISABLE_WARNING(warning) __pragma(warning(disable : warning))

///
/// @brief Wrapper around MSCV intrinsics functions
///
class MSVCCompiler : public GenericCompiler
{
public:
    /// Return the address of the current function,
    FORCEINLINE static void* ReturnAddress()
    {
        return _ReturnAddress();
    }

    /// Mark a branch as unreachable
    [[noreturn]] FORCEINLINE static void Unreachable()
    {
        __assume(false);
    }

    static std::string Demangle(const std::string_view& name);
};

/// Alias of the correct compiler currently used
using Compiler = MSVCCompiler;

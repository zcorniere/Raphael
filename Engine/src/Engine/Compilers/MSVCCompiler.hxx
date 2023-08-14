#pragma once

#include "Engine/Compilers/Compiler.hxx"
#include "Engine/Platforms/Platform.hxx"

#if !defined(COMPILER_MSVC)

static_assert(false, "MSVC Compiler header included without compiling with MSVC");

#endif

#include <intrin.h>

#define FORCEINLINE inline __forceinline   /* Force code to be inline */
#define FORCENOINLINE __declspec(noinline) /* Force code NOT to be inline */

#define ASSUME(...) __assume((__VA_ARGS__)) /* Assume code will ALWAYS be true */

///
/// @brief Wrapper around MSCV intrisics functions
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

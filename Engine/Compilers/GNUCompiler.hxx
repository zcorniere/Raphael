#pragma once

#include "Engine/Compilers/Compiler.hxx"
#include "Engine/Misc/UniquePtr.hxx"
#include "Engine/Platforms/Platform.hxx"

#include <cxxabi.h>

#if !defined(COMPILER_GNU)

static_assert(false, "GNU Compiler header included without compiling with GNU");

#endif

namespace Raphael
{

namespace Compilers
{

    ///
    /// @brief Wrapper around GCC intrisics functions
    ///
    class GNUCompiler
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

}    // namespace Compilers

/// Alias of the correct compiler currently used
using Compiler = Compilers::GNUCompiler;

}    // namespace Raphael

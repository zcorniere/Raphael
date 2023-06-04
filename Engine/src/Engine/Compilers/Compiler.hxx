#pragma once

#include <string>

#if defined(__clang__)
    #define COMPILER_CLANG
#elif defined(__GNUC__)
    #define COMPILER_GNU
#elif defined(_MSC_VER)
    #define COMPILER_MSVC
#endif

/** Branch prediction hints */
#ifndef LIKELY    // Hints compiler that expression is likely to be true
    #if (defined(COMPILER_CLANG) || defined(COMPILER_GNU))
        #define LIKELY(x) __builtin_expect(!!(x), 1)
    #else
        // the additional "!!" is added to silence "warning: equality comparison with
        // exteraneous parenthese" messages
        #define LIKELY(x) (!!(x))
    #endif
#endif

#ifndef UNLIKELY    // Hints compiler that expression is unlikely to be true
    #if (defined(COMPILER_CLANG) || defined(COMPILER_GNU))
        #define UNLIKELY(x) __builtin_expect(!!(x), 0)
    #else
        // the additional "!!" is added to silence "warning: equality comparison with
        // exteraneous parenthese" messages
        #define UNLIKELY(x) (!!(x))
    #endif
#endif

#if (defined(COMPILER_CLANG) || defined(COMPILER_GNU))
    #define FORCEINLINE inline __attribute__((always_inline)) /* Force code to be inline */
    #define FORCENOINLINE __attribute__((noinline))           /* Force code to NOT be inline */
#elif defined(COMPILER_MSVC)
    #define FORCEINLINE __forceinline /* Force code to be inline */
    #define FORCENOINLINE __declspec(noinline)
#else
    #define FORCEINLINE
    #define FORCENOINLINE
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
    [[noreturn]] FORCEINLINE static void Unreachable()
    {
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

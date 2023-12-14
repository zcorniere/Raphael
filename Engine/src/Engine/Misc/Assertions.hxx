#pragma once

#include "Engine/Compilers/Compiler.hxx"
#include "Engine/Misc/SourceLocation.hxx"
#include "Engine/Platforms/Platform.hxx"

#include <atomic>

#define RPH_CHECK_STACKTRACE

#define RPH_DEBUG_SECTION PLATFORM_CODE_SECTION(".rph_debug")

namespace Raphael
{

/// Collect and print the callstack
void RPH_DEBUG_SECTION CollectAndPrintStackTrace(void* ReturnAddress);

/// Whether or not the check should print the stacktrace
consteval RPH_DEBUG_SECTION bool ShouldCheckPrintStackTrace()
{
#if defined(RPH_CHECK_STACKTRACE)
    return true;
#else
    return false;
#endif
}

}    // namespace Raphael

#define __MACRO_EXPENDER_INTERNAL(X, Y) X##Y
#define MACRO_EXPENDER(X, Y) __MACRO_EXPENDER_INTERNAL(X, Y)

#define STR(x) #x

#ifndef NDEBUG

    #define RAPHAEL_ENSURE_IMPL(Always, Expression, Format, ...)                                                  \
        (((Expression)) || (([__VA_OPT__(&)] RPH_DEBUG_SECTION {                                                  \
                                static std::atomic_bool bExecuted = false;                                        \
                                if (!bExecuted || Always) {                                                       \
                                    bExecuted = true;                                                             \
                                    /* TODO: Check if another assertion is in progress */                         \
                                    if constexpr (Raphael::ShouldCheckPrintStackTrace())                          \
                                        Raphael::CollectAndPrintStackTrace(Compiler::ReturnAddress());            \
                                    const std::string Message = std::format(                                      \
                                        "Assertion failed: " STR(#Expression) " in {}" __VA_OPT__(" :: " Format), \
                                        file_position() __VA_OPT__(, ) __VA_ARGS__);                              \
                                    fprintf(stderr, "%s\n", Message.c_str());                                     \
                                    fflush(stderr);                                                               \
                                    return Platform::isDebuggerPresent();                                         \
                                }                                                                                 \
                                return false;                                                                     \
                            }()) &&                                                                               \
                            ([] RPH_DEBUG_SECTION {                                                               \
                                PLATFORM_BREAK();                                                                 \
                                return false;                                                                     \
                            }())))

    #define ensure(Expression) RAPHAEL_ENSURE_IMPL(false, Expression, )
    #define ensureMsg(Expression, Format, ...) RAPHAEL_ENSURE_IMPL(false, Expression, Format, ##__VA_ARGS__)
    #define ensureAlways(Expression) RAPHAEL_ENSURE_IMPL(true, Expression, )
    #define ensureAlwaysMsg(Expression, Format, ...) RAPHAEL_ENSURE_IMPL(true, Expression, Format, ##__VA_ARGS__)

    #define RAPHAEL_CHECK_IMPL(Expression, Format, ...)                                                   \
        {                                                                                                 \
            if (!(Expression)) [[unlikely]] {                                                             \
                using namespace Raphael;                                                                  \
                if constexpr (Raphael::ShouldCheckPrintStackTrace())                                      \
                    CollectAndPrintStackTrace(Compiler::ReturnAddress());                                 \
                const std::string Message =                                                               \
                    std::format("Assertion failed: " STR(#Expression) " in {}" __VA_OPT__(" :: " Format), \
                                file_position() __VA_OPT__(, ) __VA_ARGS__);                              \
                fprintf(stderr, "%s\n", Message.c_str());                                                 \
                fflush(stderr);                                                                           \
                if (Platform::isDebuggerPresent()) {                                                      \
                    PLATFORM_BREAK();                                                                     \
                } else {                                                                                  \
                    std::abort();                                                                         \
                }                                                                                         \
            }                                                                                             \
        }

    #define check(Expression) RAPHAEL_CHECK_IMPL(Expression, )
    #define checkSlow(Expression) check(Expression)
    #define checkMsg(Expression, Format, ...) RAPHAEL_CHECK_IMPL(Expression, Format, ##__VA_ARGS__)
    #define checkNoEntry()                                             \
        {                                                              \
            checkMsg(false, "Enclosing block should never be called"); \
            ::Compiler::Unreachable();                                 \
        }
    #define checkNoReentry()                                                                            \
        {                                                                                               \
            static std::atomic_bool MACRO_EXPENDER(beenHere, __LINE__) = false;                         \
            checkMsg(!MACRO_EXPENDER(beenHere, __LINE__), "Enclosing block was called more than once"); \
            MACRO_EXPENDER(beenHere, __LINE__) = true;                                                  \
        }

#else
    #define RAPHAEL_ENSURE_IMPL(Expression) (bool)(Expression)
    #define ensure(Expression) RAPHAEL_ENSURE_IMPL(Expression)
    #define ensureMsg(Expression, ...) RAPHAEL_ENSURE_IMPL(Expression)
    #define ensureAlways(Expression) RAPHAEL_ENSURE_IMPL(Expression)
    #define ensureAlwaysMsg(Expression, ...) RAPHAEL_ENSURE_IMPL(Expression)

    #define RAPHAEL_CHECK_IMPL(Expression) (void)(Expression);
    #define check(Expression) RAPHAEL_CHECK_IMPL(Expression)
    #define checkSlow(Expression)
    #define checkMsg(Expression, ...) RAPHAEL_CHECK_IMPL(Expression)
    #define checkNoEntry()             \
        {                              \
            ::Compiler::Unreachable(); \
        }
    #define checkNoReentry()                                                    \
        {                                                                       \
            static std::atomic_bool MACRO_EXPENDER(beenHere, __LINE__) = false; \
            if (MACRO_EXPENDER(beenHere, __LINE__) == true) [[unlikely]] {      \
                ::Compiler::Unreachable();                                      \
            }                                                                   \
            MACRO_EXPENDER(beenHere, __LINE__) = true;                          \
        }

#endif

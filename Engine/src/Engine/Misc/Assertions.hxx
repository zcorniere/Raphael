#pragma once

// The following header are used in the macro definitions
#include "Engine/Compilers/Compiler.hxx"     // IWYU pragma: keep
#include "Engine/Misc/SourceLocation.hxx"    // IWYU pragma: keep
#include "Engine/Platforms/Platform.hxx"

#include <atomic>    // IWYU pragma: keep

#define RPH_CHECK_STACKTRACE

namespace Raphael
{

/// Collect and print the callstack
void CollectAndPrintStackTrace(void* ReturnAddress);

/// Whether or not the check should print the stacktrace
consteval bool ShouldCheckPrintStackTrace()
{
#if defined(RPH_CHECK_STACKTRACE)
    return true;
#else
    return false;
#endif
}

}    // namespace Raphael

#ifndef NDEBUG

    #define RAPHAEL_ENSURE_IMPL(Always, Expression, Format, ...)                                                  \
        (((Expression)) || (([__VA_OPT__(&)] {                                                                    \
                                static std::atomic_bool bExecuted = false;                                        \
                                if (!bExecuted || Always) {                                                       \
                                    bExecuted.exchange(true, std::memory_order_release);                          \
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
                            ([] {                                                                                 \
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

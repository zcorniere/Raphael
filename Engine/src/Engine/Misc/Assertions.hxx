#pragma once

#include "Engine/Compilers/Compiler.hxx"
#include "Engine/Misc/SourceLocation.hxx"
#include "Engine/Platforms/Platform.hxx"

#include <atomic>

namespace Raphael
{

/// Collect and print the callstack
void CollectAndPrintStackTrace(void* ReturnAddress);

/// Whether or not the check should print the stacktrace
constexpr bool ShouldCheckPrintStackTrace()
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

    #define RAPHAEL_VERIFY_IMPL(Capture, Always, Expression, Format, ...)                                  \
        ((LIKELY(!!(Expression))) ||                                                                       \
         ([Capture]() {                                                                                    \
             static std::atomic_bool bExecuted = false;                                                    \
             if (!bExecuted || Always) {                                                                   \
                 bExecuted = true;                                                                         \
                 /* TODO: Check if another assertion is in progress */                                     \
                 if (Raphael::ShouldCheckPrintStackTrace())                                                \
                     Raphael::CollectAndPrintStackTrace(Compiler::ReturnAddress());                        \
                 const std::string Message =                                                               \
                     std::format("Assertion failed: " STR(#Expression) " in {}" __VA_OPT__(" :: " Format), \
                                 file_position() __VA_OPT__(, ) __VA_ARGS__);                              \
                 fprintf(stderr, "%s\n", Message.c_str());                                                 \
                 fflush(stderr);                                                                           \
                 return Platform::isDebuggerPresent();                                                     \
             }                                                                                             \
             return false;                                                                                 \
         }()) &&                                                                                           \
             ([]() { PLATFORM_BREAK(); }(), false))

    #define verify(Expression) RAPHAEL_VERIFY_IMPL(, false, Expression, )
    #define verifyMsg(Expression, Format, ...) RAPHAEL_VERIFY_IMPL(&, false, Expression, Format, ##__VA_ARGS__)
    #define verifyAlways(Expression) RAPHAEL_VERIFY_IMPL(, true, Expression, )
    #define verifyAlwaysMsg(Expression, Format, ...) RAPHAEL_VERIFY_IMPL(&, true, Expression, Format, ##__VA_ARGS__)

    #define RAPHAEL_CHECK_IMPL(Expression, Format, ...)                                                   \
        {                                                                                                 \
            if (UNLIKELY(!(Expression))) {                                                                \
                using namespace Raphael;                                                                  \
                if (Raphael::ShouldCheckPrintStackTrace())                                                \
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

    #define verify(Expression) (LIKELY(!!(Expression)))
    #define verifyMsg(Expression, ...) (LIKELY(!!(Expression)))
    #define verifyAlways(Expression) (LIKELY(!!(Expression)))
    #define verifyAlwaysMsg(Expression, ...) (LIKELY(!!(Expression)))

    #define check(Expression) (LIKELY(!!(Expression)))
    #define checkSlow(Expression)
    #define checkMsg(Expression, ...) (LIKELY(!!(Expression)))
    #define checkNoEntry()             \
        {                              \
            ::Compiler::Unreachable(); \
        }
    #define checkNoReentry()                                                    \
        {                                                                       \
            static std::atomic_bool MACRO_EXPENDER(beenHere, __LINE__) = false; \
            if (UNLIKELY(MACRO_EXPENDER(beenHere, __LINE__) == true)) {         \
                ::Compiler::Unreachable();                                      \
            }                                                                   \
            MACRO_EXPENDER(beenHere, __LINE__) = true;                          \
        }

#endif

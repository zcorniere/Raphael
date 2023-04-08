#pragma once

#include "Engine/Compilers/Compiler.hxx"
#include "Engine/Misc/SourceLocation.hxx"
#include "Engine/Platforms/Platform.hxx"

#include <atomic>

void CollectAndPrintStackTrace(void *ReturnAddress);

#define MACRO_EXPENDER_INTERNAL(X, Y) X##Y
#define MACRO_EXPENDER(X, Y) PIVOT_MACRO_EXPENDER_INTERNAL(X, Y)

#define STR(x) #x

#ifndef NDEBUG

    #define RAPHAEL_VERIFY_IMPL(Capture, Always, Expression, Format, ...)                                  \
        ((LIKELY(!!(Expression))) || ([Capture]() {                                                        \
                                         static std::atomic_bool bExecuted = false;                        \
                                         if (!bExecuted || Always) {                                       \
                                             bExecuted = true;                                             \
                                             /* TODO: Check if another assertion is in progress */         \
                                             CollectAndPrintStackTrace(Compiler::ReturnAddress());         \
                                             cpplogger::fmt::print(stderr, "Assertion failed:" STR(#Expression)          \
                                                                 __VA_OPT__(" :: " Format, ) __VA_ARGS__); \
                                             cpplogger::fmt::print(stderr, "\n");                                        \
                                             return Platform::isDebuggerPresent();                         \
                                         }                                                                 \
                                         return false;                                                     \
                                     }()) &&                                                               \
                                         ([]() { PLATFORM_BREAK(); }(), false))

    #define verify(Expression) RAPHAEL_VERIFY_IMPL(, false, Expression, )
    #define verifyMsg(Expression, Format, ...) RAPHAEL_VERIFY_IMPL(&, false, Expression, Format, ##__VA_ARGS__)
    #define verifyAlways(Expression) RAPHAEL_VERIFY_IMPL(, true, Expression, )
    #define verifyAlwaysMsg(Expression, Format, ...) RAPHAEL_VERIFY_IMPL(&, true, Expression, Format, ##__VA_ARGS__)

    #define RAPHAEL_CHECK_IMPL(Expression, ...)                                                        \
        {                                                                                              \
            if (UNLIKELY(!(Expression))) {                                                             \
                CollectAndPrintStackTrace(Compiler::ReturnAddress());                                  \
                cpplogger::fmt::print(stderr, "Assertion failed: " STR(#Expression) __VA_OPT__(" :: ") __VA_ARGS__); \
                cpplogger::fmt::print(stderr, "\n");                                                                 \
                fflush(stderr);                                                                        \
                if (Platform::isDebuggerPresent()) { PLATFORM_BREAK(); }                               \
                std::abort();                                                                          \
            }                                                                                          \
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
    #define checkNoEntry()                      \
        {                                       \
            ::Compiler::Unreachable(); \
        }
    #define checkNoReentry()                                                                        \
        {                                                                                           \
            static std::atomic_bool MACRO_EXPENDER(beenHere, __LINE__) = false;                     \
            if (MACRO_EXPENDER(beenHere, __LINE__) == true) { ::Raphael::Compiler::Unreachable(); } \
            MACRO_EXPENDER(beenHere, __LINE__) = true;                                              \
        }

#endif

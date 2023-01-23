#pragma once

#include "Engine/Compilers/Compiler.hxx"
#include "Engine/Misc/SourceLocation.hxx"
#include "Engine/Platforms/Platform.hxx"

#include <atomic>

namespace Raphael::Assertions
{

void CollectAndPrintStackTrace(void *ReturnAddress);
}

#define MACRO_EXPENDER_INTERNAL(X, Y) X##Y
#define MACRO_EXPENDER(X, Y) PIVOT_MACRO_EXPENDER_INTERNAL(X, Y)

#define STR(x) #x

#ifndef NDEBUG

    #define RAPHAEL_VERIFY_IMPL(Capture, Always, Expression, Format, ...)                                      \
        ((LIKELY(!!(Expression))) || ([Capture]() {                                                            \
                                         using namespace Raphael;                                              \
                                         static std::atomic_bool bExecuted = false;                            \
                                         if (!bExecuted || Always) {                                           \
                                             bExecuted = true;                                                 \
                                             /* TODO: Check if another assertion is in progress */             \
                                             Assertions::CollectAndPrintStackTrace(Compiler::ReturnAddress()); \
                                             fprintf(stderr, "Assertion failed:" STR(#Expression)              \
                                                                 __VA_OPT__(" :: " Format, ) __VA_ARGS__);     \
                                             fprintf(stderr, "\n");                                            \
                                             return Platform::isDebuggerPresent();                             \
                                         }                                                                     \
                                         return false;                                                         \
                                     }()) &&                                                                   \
                                         ([]() { PLATFORM_BREAK(); }(), false))

    #define verify(Expression) RAPHAEL_VERIFY_IMPL(, false, Expression, )
    #define verifyMsg(Expression, Format, ...) RAPHAEL_VERIFY_IMPL(&, false, Expression, Format, ##__VA_ARGS__)
    #define verifyAlways(Expression) RAPHAEL_VERIFY_IMPL(, true, Expression, )
    #define verifyAlwaysMsg(Expression, Format, ...) RAPHAEL_VERIFY_IMPL(&, true, Expression, Format, ##__VA_ARGS__)

    #define RAPHAEL_CHECK_IMPL(Expression, Format, ...)                                                        \
        {                                                                                                      \
            using namespace Raphael;                                                                           \
                                                                                                               \
            if (UNLIKELY(!(Expression))) {                                                                     \
                Assertions::CollectAndPrintStackTrace(Compiler::ReturnAddress());                              \
                fprintf(stderr, "Assertion failed:" STR(#Expression) __VA_OPT__(" :: " Format, ) __VA_ARGS__); \
                fflush(stderr);                                                                                \
                if (Platform::isDebuggerPresent()) { PLATFORM_BREAK(); }                                       \
                std::abort();                                                                                  \
            }                                                                                                  \
        }

    #define check(Expression) RAPHAEL_CHECK_IMPL(Expression, )
    #define checkMsg(Expression, Format, ...) RAPHAEL_CHECK_IMPL(Expression, Format, ##__VA_ARGS__)
    #define checkNoEntry()                                             \
        {                                                              \
            using namespace Raphael;                                   \
                                                                       \
            checkMsg(false, "Enclosing block should never be called"); \
            Compiler::Unreachable();                                   \
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

    #define check(Expression)
    #define checkMsg(Expression, ...)
    #define checkNoEntry()
    #define checkNoReentry()

#endif

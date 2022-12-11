#pragma once

#include "Engine/Compilers/Compiler.hxx"
#include "Engine/Misc/SourceLocation.hxx"
#include "Engine/Platforms/Platform.hxx"

#include <atomic>

#define MACRO_EXPENDER_INTERNAL(X, Y) X##Y
#define MACRO_EXPENDER(X, Y) PIVOT_MACRO_EXPENDER_INTERNAL(X, Y)

#define STR(x) #x

#ifndef NDEBUG

    #define RAPHAEL_VERIFY_IMPL(Capture, Always, Expression, ...)                                              \
        ((LIKELY(!!(Expression))) || ([Capture](const std::string FunctionName = Utils::function_name()) {     \
                                         using namespace Raphael;                                              \
                                         static std::atomic_bool bExecuted = false;                            \
                                         if (!bExecuted || Always) {                                           \
                                             bExecuted = true;                                                 \
                                             logger.err(FunctionName) << "Assertion failed: " STR(#Expression) \
                                                     __VA_OPT__(" :: " <<) __VA_ARGS__;                        \
                                             return Platform::isDebuggerPresent();                             \
                                         }                                                                     \
                                         return false;                                                         \
                                     }()) &&                                                                   \
                                         ([]() { Raphael::Platform::breakpoint(); }(), false))

    #define verify(Expression) RAPHAEL_VERIFY_IMPL(, false, Expression)
    #define verifyMsg(Expression, ...) RAPHAEL_VERIFY_IMPL(&, false, Expression, ##__VA_ARGS__)
    #define verifyAlways(Expression) RAPHAEL_VERIFY_IMPL(, true, Expression)
    #define verifyAlwaysMsg(Expression, ...) RAPHAEL_VERIFY_IMPL(&, true, Expression, ##__VA_ARGS__)

    #define RAPHAEL_ASSERT_IMPL(Always, Expression, ...)                                        \
        {                                                                                       \
            using namespace Raphael;                                                            \
                                                                                                \
            if (UNLIKELY(!(Expression))) {                                                      \
                logger.err(Utils::function_name())                                              \
                    << "Assertion failed: " STR(#Expression) __VA_OPT__(" :: " <<) __VA_ARGS__; \
                logger.stop();                                                                  \
                if (Platform::isDebuggerPresent()) { Platform::breakpoint(); }                  \
                std::abort();                                                                   \
            }                                                                                   \
        }

    #define check(Expression) RAPHAEL_ASSERT_IMPL(false, Expression)
    #define checkMsg(Expression, ...) RAPHAEL_ASSERT_IMPL(false, Expression, ##__VA_ARGS__)
    #define checkNoEntry()                                             \
        {                                                              \
            using namespace Raphael;                                   \
                                                                       \
            checkMsg(false, "Enclosing block should never be called"); \
            Compiler::unreachable();                                   \
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

#pragma once

// The following header are used in the macro definitions
#include "Engine/Compilers/Compiler.hxx"    // IWYU pragma: keep
#include "Engine/Platforms/Platform.hxx"

#include <atomic>    // IWYU pragma: keep

#define RPH_CHECK_STACKTRACE

namespace Raphael::Debug
{

bool HandleCheckFailure(const std::string& Message, bool bShouldAbort);

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

#ifndef NDEBUG
class FRecursionScopeMarker
{
public:
    FRecursionScopeMarker(uint16& InCounter): Counter(InCounter)
    {
        ++Counter;
    }
    ~FRecursionScopeMarker()
    {
        --Counter;
    }

private:
    uint16& Counter;
};
#endif    // !NDEBUG

}    // namespace Raphael::Debug

#ifndef NDEBUG

    #define RAPHAEL_ENSURE_IMPL(Always, Expression, Format, ...) \
        (((Expression)) || (([__VA_OPT__(&)] {                                                             \
                                static std::atomic_bool bExecuted = false;                                 \
                                if (!bExecuted || Always) {                                                \
                                    bExecuted.exchange(true, std::memory_order_release);                   \
                                                                                                           \
                                    const std::string Message =                                            \
                                        std::format("Assertion failed: " STR(#Expression) " in {}" Format, \
                                                    ::RTTI::FilePosition() __VA_OPT__(, ) __VA_ARGS__);    \
                                    return ::Raphael::Debug::HandleCheckFailure(Message, false);           \
                                }                                                                          \
                                return false;                                                              \
                            }()) &&                                                                        \
                            ([] {                                                                          \
                                PLATFORM_BREAK();                                                          \
                                return false;                                                              \
                            }())))

    #define ensure(Expression) RAPHAEL_ENSURE_IMPL(false, Expression, )
    #define ensureMsg(Expression, Format, ...) RAPHAEL_ENSURE_IMPL(false, Expression, " :: " Format, ##__VA_ARGS__)
    #define ensureAlways(Expression) RAPHAEL_ENSURE_IMPL(true, Expression, )
    #define ensureAlwaysMsg(Expression, Format, ...) RAPHAEL_ENSURE_IMPL(true, Expression, " :: " Format, ##__VA_ARGS__)

    #define RAPHAEL_CHECK_IMPL(Expression, Format, ...)                                                        \
        {                                                                                                      \
            if (!(Expression)) [[unlikely]]                                                                    \
            {                                                                                                  \
                const std::string Message = std::format("Assertion failed: " STR(#Expression) " in {}" Format, \
                                                        ::RTTI::FilePosition() __VA_OPT__(, ) __VA_ARGS__);    \
                ::Raphael::Debug::HandleCheckFailure(Message, true);                                           \
            }                                                                                                  \
        }

    #define check(Expression) RAPHAEL_CHECK_IMPL(Expression, )
    #define checkSlow(Expression) check(Expression)
    #define checkMsg(Expression, Format, ...) RAPHAEL_CHECK_IMPL(Expression, " :: " Format, ##__VA_ARGS__)
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

    #define checkNoRecursion()                                                                                \
        static uint16 MACRO_EXPENDER(RecursionCounter, __LINE__) = 0;                                         \
        checkMsg(MACRO_EXPENDER(RecursionCounter, __LINE__) == 0, "Enclosing block was entered recursively"); \
        const ::Raphael::RecursionScopeMarker MACRO_EXPENDER(ScopeMarker,                                     \
                                                             __LINE__)(MACRO_EXPENDER(RecursionCounter, __LINE__))

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
            if (MACRO_EXPENDER(beenHere, __LINE__) == true) [[unlikely]]        \
            {                                                                   \
                ::Compiler::Unreachable();                                      \
            }                                                                   \
            MACRO_EXPENDER(beenHere, __LINE__) = true;                          \
        }
    #define checkNoRecursion()

#endif

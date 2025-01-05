#pragma once

#include <cstddef>
#include <tuple>

// Thanks to https://stackoverflow.com/questions/7943525/
// is-it-possible-to-figure-out-the-parameter-type-and-return-type-of-a-lambda?rq=1

template <typename>
struct function_traits;

template <typename T>
struct function_traits : public function_traits<decltype(&T::operator())> {
};

// Specialization for pointers to member function or lambdas
template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType (ClassType::*)(Args...) const> : public function_traits<ReturnType (*)(Args...)> {
};

// Specialization for function pointers
template <typename ReturnType, typename... Args>
struct function_traits<ReturnType (*)(Args...)> {
    static constexpr size_t arity = sizeof...(Args);

    using result_type = ReturnType;

    using args_type = std::tuple<Args...>;

    template <size_t i>
    struct arg {
        using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
    };
};

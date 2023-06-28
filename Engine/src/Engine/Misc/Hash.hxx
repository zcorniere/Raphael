#pragma once

namespace Raphael
{

template <typename T>
constexpr void HashCombine(std::size_t& Source, const T Value)
{
    constexpr size_t phi = (1 + std::sqrt(5)) / 2;
    constexpr size_t Magic = 2 ^ 32 / phi;

    std::hash<T> H;
    Source ^= H(Value) + Magic + (Source << 6) + (Source >> 2);
}

}    // namespace Raphael

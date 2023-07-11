#pragma once

template <typename T>
struct Wrapper
{
    using type = T;
    T value{};
    constexpr Wrapper(const T value) : value{value} {}
    constexpr Wrapper() {}
    constexpr explicit operator T() { return value; }
};
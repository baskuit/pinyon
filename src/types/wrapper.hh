#pragma once

template <typename T>
struct Wrapper
{
    using type = T;
    T value{};
    constexpr Wrapper() {}
    constexpr Wrapper(const T value) : value{value} {}
    constexpr explicit operator T() const { return value; }
};
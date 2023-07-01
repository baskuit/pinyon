#pragma once

template <typename T>
struct Wrapper
{
    T value{};
    constexpr Wrapper(const T value) : value{value} {}
    constexpr Wrapper() {}
    constexpr explicit operator T() { return value; }
};
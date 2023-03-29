#pragma once

#include <vector>
#include <algorithm>

template <typename T>
struct Vector : std::vector<T>
{

    Vector() {}

    Vector(size_t n)
    {
        this->resize(n);
    }

    void fill(int n, T value)
    {
        this->resize(n);
        std::fill(this->begin(), this->begin() + n, value);
    }
};

template <typename T, size_t size>
struct Array : std::array<T, size>
{

    Array() {}

    Array(size_t n)
    {
    }

    void fill(int n, T value)
    {
        std::fill(this->begin(), this->begin() + n, value);
    }
};
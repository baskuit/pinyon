#pragma once

#include <vector>
#include <array>
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

    void fill(int n)
    {
        this->resize(n);
    }
};

template <typename T, size_t size>
struct Array : std::array<T, size>
{

    Array() {}

    Array(size_t n)
    {
    }

    int size = 0;

    void fill(int n, T value)
    {
        std::fill(this->begin(), this->begin() + n, value);
        size = n;
    }
    void fill(int n)
    {
        size = n;
    }
    int size () {
        return size;
    }
};
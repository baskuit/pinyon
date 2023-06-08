#pragma once

#include <vector>
#include <array>
#include <algorithm>

template <typename T>
struct Vector : std::vector<T>
{

    Vector() {}

    Vector(const std::vector<T> &vec) : std::vector<T>(vec) {}

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

template <typename T, size_t MaxSize>
struct Array : std::array<T, MaxSize>
{

    Array() {}

    Array(size_t n)
    {
    }

    int _size = 0; // TODO does array already have a size member?

    void fill(int n, T value)
    {
        std::fill(this->begin(), this->begin() + n, value);
        _size = n;
    }
    void fill(int n)
    {
        _size = n;
    }
    int size () {
        return _size;
    }
};
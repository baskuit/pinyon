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

    void fill(int n)
    {
        this->resize(n);
    }
};

template <typename T>
void fill(std::vector<T> &vector, int n, T value)
{
    vector.resize(n);
    std::fill(vector.begin(), vector.begin() + n, value);
}

template <typename T>
void fill(std::vector<T> &vector, int n)
{
    vector.resize(n);
}

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
    void fill(int n)
    {
    }
};

template <typename T, int size>
void fill(std::array<T, size> &vector, int n, T value)
{
    std::fill(vector.begin(), vector.begin() + n, value);
}

template <typename T, int size>
void fill(std::array<T, size> &vector, int n)
{
}
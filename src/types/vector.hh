#pragma once

#include <vector>
#include <array>
#include <algorithm>

#include "wrapper.hh"

template <typename T>
struct Vector : std::vector<T>
{

    Vector() {}

    Vector(const std::vector<T> &vec) : std::vector<T>(vec) {}

    Vector(size_t n)
    {
        this->resize(n);
    }

    void fill(size_t n, T value)
    {
        this->resize(n);
        std::fill(this->begin(), this->begin() + n, value);
    }

    void fill(size_t n)
    {
        this->resize(n);
    }
};

template <size_t MaxSize>
struct A
{
    template <typename T>
    struct Array : std::array<T, MaxSize>
    {

        Array() {}

        Array(size_t n)
        {
        }

        size_t _size = 0;

        void fill(size_t n, T value)
        {
            std::fill(this->begin(), this->begin() + n, value);
            _size = n;
        }

        void fill(size_t n)
        {
            _size = n;
        }

        size_t size()
        {
            return _size;
        }

        std::array<T, MaxSize>::iterator end()
        {
            return std::array<T, MaxSize>::begin() + _size;
        }

        std::array<T, MaxSize>::const_iterator end() const
        {
            return std::array<T, MaxSize>::begin() + _size;
        }

        template <typename U, template <typename U_> typename W>
        U *data()
        {
            return reinterpret_cast<U *>(this->std::array<T, MaxSize>::data());
        }
    };
};
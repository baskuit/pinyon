#pragma once

#include <array>
#include <algorithm>

template <size_t MaxSize>
struct A
{
    template <typename T>
    struct Array : std::array<T, MaxSize>
    {

        Array() {}

        Array(const Array &other)
        {
            std::copy(other.begin(), other.end(), this->begin());
        }

        Array &operator=(const Array &other)
        {
            std::copy(other.begin(), other.end(), this->begin());
            return *this;
        }

        size_t _size = 0;

        void resize(size_t n, T value)
        {
            _size = n;
            std::fill(this->begin(), this->end(), value);
        }

        void resize(size_t n)
        {
            _size = n;
        }

        size_t size() const
        {
            return _size;
        }

        std::array<T, MaxSize>::iterator end()
        {
            return std::array<T, MaxSize>::begin() + _size;
        }

        const std::array<T, MaxSize>::const_iterator end() const
        {
            return std::array<T, MaxSize>::begin() + _size;
        }
    };
};
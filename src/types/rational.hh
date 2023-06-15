#pragma once

#include <iostream>

template <typename T = int>
struct Rational
{

    T p = 1;
    T q = 1;

    void reduce()
    {
        // eulcid's algorithm
        T a = p;
        T b = q;
        T c;
        while (b)
        {
            a %= b;
            c = a;
            a = b;
            b = c;
        }
        p /= a;
        q /= a;
    }

public:

    constexpr Rational() {}

    constexpr Rational(const T p) : p(p), q(1) {}

    constexpr Rational(const T p, const T q) : p(p), q(q) {}

    constexpr Rational operator+(const Rational y) const
    {
        Rational z = {p * y.q + y.p * q, q * y.q};
        z.reduce();
        return z;
    }

    constexpr Rational operator*(const Rational y) const
    {
        Rational z = {p * y.p, q * y.q};
        z.reduce();
        return z;
    }

    constexpr Rational operator/(const Rational y) const
    {
        Rational z = {p * y.q, q * y.p};
        z.reduce();
        return z;
    }

    constexpr bool operator<(const Rational y) const
    {
        return p * y.q < y.p * q;
    }

    constexpr bool operator<=(Rational y)
    {
        return p * y.q <= y.p * q;
    }

    constexpr bool operator>(Rational y)
    {
        return p * y.q > y.p * q;
    }

    constexpr bool operator>=(Rational y)
    {
        return p * y.q >= y.p * q;
    }

    constexpr Rational &operator+=(Rational y)
    {
        p = p * y.q + y.p * q;
        q = p * y.q;
        reduce();
        return *this;
    }

    friend std::ostream &operator<<(std::ostream &os, const Rational &x)
    {
        os << x.p << '/' << x.q;
        return os;
    }

    constexpr operator float() const
    {
        return p / (float)q;
    }

    constexpr operator double() const 
    {
        return p / (double)q;
    }
};

template <typename U, typename T>
bool operator<(U x, const Rational<T>& y)
{
    return x * y.q < y.p;
}

template <typename U, typename T>
bool operator>(U x, const Rational<T>& y)
{
    return x * y.q > y.p;
}

template <template <typename U> class V, typename U, typename T>
bool operator>(V<U> x, const Rational<T>& y)
{
    return x.value * y.q > y.p;
}
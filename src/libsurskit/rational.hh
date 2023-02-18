#pragma once

#include <iostream>

class Rational
{

    int p = 1;
    int q = 1;

    void reduce()
    {
        // eulcid's algorithm
        int a = p;
        int b = q;
        int c;
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
    Rational() {}

    Rational(int p, int q) : p(p), q(q) {}

    Rational operator+(Rational y)
    {
        Rational z = {p * y.q + y.p * q, q * y.q};
        z.reduce();
        return z;
    }

    Rational operator*(Rational y)
    {
        Rational z = {p * y.p, q * y.q};
        z.reduce();
        return z;
    }

    Rational operator/(Rational y)
    {
        Rational z = {p * y.q, q * y.p};
        z.reduce();
        return z;
    }

    bool operator<(Rational y)
    {
        return p * y.q < y.p * q;
    }

    bool operator<=(Rational y)
    {
        return p * y.q <= y.p * q;
    }

    bool operator>(Rational y)
    {
        return p * y.q > y.p * q;
    }

    bool operator>=(Rational y)
    {
        return p * y.q >= y.p * q;
    }

    Rational &operator+=(Rational y)
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

    operator float()
    {
        return p / (float)q;
    }

    operator double()
    {
        return p / (double)q;
    }
};
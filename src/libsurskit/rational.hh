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

    Rational(int p) : p(p), q(1) {}

    Rational(int p, int q) : p(p), q(q) {}

    Rational(double x)
    {
        p = 0;
        q = 1;
        if (x != 0.0) {
            bool neg = x < 0;
            if (neg) {
                x = -x;
            }

            constexpr long shift = 15;         // a safe shift per step
            constexpr double width = 1 << shift;  // = 2^shift
            const int maxiter = 20;        // ought not be necessary, but just in case,
            // max 300 bits of precision
            int expt;
            double mantissa = frexp(x, &expt);
            long exponent = expt;
            double intpart;
            int k = 0;
            while (mantissa != 0.0 && k++ < maxiter)
            {
                mantissa *= width;
                mantissa = modf(mantissa, &intpart);
                p <<= shift;
                p += (long)intpart;
                exponent -= shift;
            }
            if (exponent > 0)
                p <<= exponent;
            else if (exponent < 0)
                q <<= -exponent;
            if (neg) {
                p *= -1;
            }
        }
        reduce();
    }

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
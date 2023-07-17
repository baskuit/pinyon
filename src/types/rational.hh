#pragma once

#include <gmpxx.h>

#include <iostream>

template <typename T = int>
struct Rational
{

    T p = 1;
    T q = 1;

    operator mpq_class() const
    {
        std::cout << "rational" << std::endl;
        return mpq_class{p, q};
    } 
};
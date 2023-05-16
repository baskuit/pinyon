#pragma once

#include "libsurskit/rational.hh"
#include "libsurskit/vector.hh"
#include "libsurskit/random.hh"

#include <vector>

template <
    typename _Rational,
    typename _Real,
    typename _Float,
    typename _Action,
    typename _Observation,
    typename _Seed,
    typename _PRNG,
    typename _VectorReal,
    typename _VectorAction,
    typename _VectorInt,
    typename _MatrixReal,
    typename _MatrixFloat,
    typename _MatrixInt
>
struct Types {

    using Rational = _Rational;
    using Real = _Real;
    using Float = _Float;
    using Action = _Action;
    using Observation = _Observation;
    using Seed = _Seed;
    using PRNG = _PRNG;
    using VectorReal = _VectorReal;
    using VectorAction = _VectorAction;
    using VectorInt = _VectorInt;
    using MatrixReal = _MatrixReal;
    using MatrixFloat = _MatrixFloat;
    using MatrixInt = _MatrixInt;
};

using SimpleTypes = Types<
    Rational<int>,
    double,
    double,
    int,
    int,
    uint64_t,
    prng,
    Vector<double>,
    Vector<int>,
    Vector<int>,
    Linear::MatrixVector<double>,
    Linear::MatrixVector<double>,
    Linear::MatrixVector<int>
>;
    
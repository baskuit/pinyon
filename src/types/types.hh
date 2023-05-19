#pragma once

#include "../libsurskit/rational.hh"
#include "../libsurskit/vector.hh"
#include "../libsurskit/random.hh"
#include "../libsurskit/math.hh"
#include "arithmetic.hh"

#include <vector>

using ActionIndex = int;

template <
    typename _Rational,
    typename _Real,
    typename _Float,
    typename _Action,
    typename _Observation,
    typename _Probability,
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

    // template <typename T>
    // struct RationalType : ArithmeticType<T> {
    //     explicit RationalType(T val) : ArithmeticType<T>{val} {}
    //     explicit RationalType () : ArithmeticType<T>{} {}
    // };
    // Rational is basically a primitive, yeah?

    template <typename T>
    struct RealType : ArithmeticType<T> {
        explicit RealType(T val) : ArithmeticType<T>{val} {}
        explicit RealType () : ArithmeticType<T>{} {}
    };

    template <typename T>
    struct FloatType : ArithmeticType<T> {
        explicit FloatType(T val) : ArithmeticType<T>{val} {}
        explicit FloatType () : ArithmeticType<T>{} {}
    };

    template <typename T>
    struct ActionType {
        T value;
    };

    template <typename T>
    struct ObservationType {
        T value;
    };

    template <typename T>
    struct ProbabilityType : ArithmeticType<T> {
        explicit ProbabilityType(T val) : ArithmeticType<T>{val} {}
        explicit ProbabilityType () : ArithmeticType<T>{} {}
    };

    template <typename T>
    struct Vector;

    using Rational = _Rational;
    using Real = RealType<_Real>;
    using Float = _Float;
    using Action = _Action;
    using Observation = _Observation;
    using Probability = _Probability;
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
    double,
    uint64_t,
    prng,
    Vector<double>,
    Vector<int>,
    Vector<int>,
    Linear::Matrix<double>,
    Linear::Matrix<double>,
    Linear::Matrix<int>
>;
    
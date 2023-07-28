#pragma once

#include <libsurskit/math.hh>
#include <types/wrapper.hh>
#include <types/random.hh>
#include <types/array.hh>
#include <types/matrix.hh>
#include <types/random.hh>
#include <types/value.hh>
#include <types/mutex.hh>

/*

TypeList

*/

template <
    typename _Real,
    typename _Action,
    typename _Observation,
    typename _Probability,

    template <typename...> typename _Value = PairReal,
    template <typename...> typename _Vector = std::vector,
    template <typename...> typename _Matrix = Matrix,

    typename _Mutex = std::mutex,
    typename _Seed = uint64_t,
    typename _PRNG = prng,
    typename _Rational = Rational<int>>
struct DefaultTypes
{
    using Real = RealType<_Real>;
    using Action = ActionType<_Action>;
    using Observation = ObservationType<_Observation>;
    using Probability = ProbabilityType<_Probability>;

    using Value = _Value<Real>;
    using VectorReal = _Vector<Real>;
    using VectorAction = _Vector<Action>;
    using VectorInt = _Vector<int>;
    using MatrixReal = _Matrix<Real>;
    using MatrixInt = _Matrix<int>;
    using MatrixValue = _Matrix<Value>;
    template <typename... Args>
    using Vector = _Vector<Args...>;
    template <typename... Args>
    using Matrix = _Matrix<Args...>;

    using ObservationHash = ObservationHashType<_Observation>;
    using Mutex = std::mutex;
    using Seed = _Seed;
    using PRNG = _PRNG;
    using Rational = _Rational;
};

using SimpleTypes = DefaultTypes<
    double,
    int,
    int,
    double>;

using RandomTreeFloatTypes = DefaultTypes<
    double,
    int,
    int,
    double,
    ConstantSum<1, 1>::Value>;

using RandomTreeRationalTypes = DefaultTypes<
    mpq_class,
    int,
    int,
    mpq_class,
    ConstantSum<1, 1>::Value>;

/*

Concepts

*/

template <typename Types>
concept IsTypeList = requires(Types obj) {
    typename Types::Real;
    typename Types::Action;
    typename Types::Observation;
    typename Types::Probability;
    typename Types::Value;
    typename Types::VectorReal;
    typename Types::VectorAction;
    typename Types::VectorInt;
    typename Types::MatrixReal;
    typename Types::MatrixValue;
    typename Types::MatrixInt;
    typename Types::Mutex;
    typename Types::PRNG;
    typename Types::Seed;
    typename Types::Rational;
};

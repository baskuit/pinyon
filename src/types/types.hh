#pragma once

#include <libsurskit/math.hh>
#include <types/wrapper.hh>
#include <types/random.hh>
#include <types/array.hh>
#include <types/matrix.hh>
#include <types/random.hh>
#include <types/value.hh>
#include <types/mutex.hh>

template <class Class, typename Types>
struct AddTypes;

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

template <typename Real>
concept IsArithmetic = requires(Real real) {
    static_cast<Real>(real + real);
    {
        real.canonicalize()
    } -> std::same_as<void>;
};

template <typename Value, typename Real>
concept IsValue = requires(Value value) {
    {
        value.get_row_value()
    } -> std::same_as<Real>;
    {
        value.get_col_value()
    } -> std::same_as<Real>;
};

template <typename Types>
concept IsTypeList =
    requires(
        Types obj,
        typename Types::Action &action,
        typename Types::Observation &obs,
        typename Types::VectorReal &strategy,
        typename Types::VectorAction &actions,
        typename Types::VectorInt &visits,
        typename Types::MatrixReal &real_matrix,
        typename Types::MatrixValue &payoff_matrix,
        typename Types::MatrixInt &visit_matrix) {
        {
            obs == obs
        } -> std::same_as<bool>;
        {
            strategy[0]
        } -> std::same_as<typename Types::Real &>;
        {
            actions[0]
        } -> std::same_as<typename Types::Action &>;
        {
            visits[0]
        } -> std::same_as<int &>;
        {
            real_matrix.get(0, 0)
        } -> std::same_as<typename Types::Real &>;
        {
            payoff_matrix.get(0, 0)
        } -> std::same_as<typename Types::Value &>;
        {
            visit_matrix.get(0, 0)
        } -> std::same_as<int &>;
    } &&
    IsArithmetic<typename Types::Real> &&
    IsArithmetic<typename Types::Probability> &&
    IsValue<typename Types::Value, typename Types::Real>;

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
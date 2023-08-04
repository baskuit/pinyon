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
    typename _Obs,
    typename _Prob,

    template <typename...> typename _Value = PairReal,
    template <typename...> typename _Vector = std::vector,
    template <typename...> typename _Matrix = Matrix,

    typename _Mutex = std::mutex,
    typename _Seed = uint64_t,
    typename _PRNG = prng,
    typename _Rational = Rational<int>>
struct DefaultTypes
{
    using TypeList = DefaultTypes;
    using Real = RealType<_Real>;
    using Action = ActionType<_Action>;
    using Obs = ObsType<_Obs>;
    using Prob = ProbType<_Prob>;

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

    using ObsHash = ObsHashType<_Obs>;
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
concept IsValue = requires(Value &value) {
    {
        value.get_row_value()
    } -> std::same_as<Real>;
    {
        value.get_col_value()
    } -> std::same_as<Real>;
    {
        value = value
        // copy assignable
    } -> std::same_as<Value &>;
    // value = value;
};

template <typename Mutex>
concept IsMutex = requires(Mutex &mutex) {
    {
        mutex.lock()
    } -> std::same_as<void>;
    {
        mutex.unlock()
    } -> std::same_as<void>;
};

template <typename Vector, typename T>
concept IsVector = requires(Vector &vector, T &value) {
    {
        vector[0]
    } -> std::same_as<T &>;
    {
        vector.resize(0)
    } -> std::same_as<void>;
    {
        vector.resize(0, value)
    } -> std::same_as<void>;
    {
        vector.clear()
    } -> std::same_as<void>;
} && std::ranges::sized_range<Vector>;

template <typename PRNG, typename Seed>
concept IsPRNG = requires(PRNG &device, const PRNG &const_device, Seed seed) {
    {
        device = const_device
        // copy constructable
    };
    {
        device.random_int(0)
    } -> std::convertible_to<int>;
    {
        device.uniform()
    } -> std::same_as<double>;
    {
        device.discard(0)
    } -> std::same_as<void>;
    {
        device.sample_pdf(std::vector<PRNG>{})
        // asserts PRNG is default constructable and sample_pdf exists universally (surely) for all vector value types
    } -> std::convertible_to<int>;
    {
        device.get_seed()
    } -> std::same_as<Seed>;
    PRNG{seed};
};

template <typename Types>
concept IsTypeList =
    requires(
        Types obj,
        typename Types::Action &action,
        typename Types::Obs &obs,
        typename Types::MatrixReal &real_matrix,
        typename Types::MatrixValue &payoff_matrix,
        typename Types::MatrixInt &visit_matrix) {
        {
            obs == obs
        } -> std::same_as<bool>;
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
    IsArithmetic<typename Types::Prob> &&
    IsValue<typename Types::Value, typename Types::Real> &&
    IsPRNG<typename Types::PRNG, typename Types::Seed> &&
    IsMutex<typename Types::Mutex> &&
    IsVector<typename Types::VectorReal, typename Types::Real> &&
    IsVector<typename Types::VectorInt, int> &&
    IsVector<typename Types::VectorAction, typename Types::Action>;

/*

Instances

*/

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
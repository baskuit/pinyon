#pragma once

#include <libsurskit/math.hh>
#include <types/random.hh>
#include <types/vector.hh>
#include <types/matrix.hh>
#include <types/random.hh>
#include <types/arithmetic.hh>
#include <types/value.hh>
#include <types/wrapper.hh>
#include <types/mutex.hh>

#include <vector>

template <typename T>
struct RealType : ArithmeticType<T>
{
    constexpr RealType() : ArithmeticType<T>{} {}
    constexpr RealType(const T &val) : ArithmeticType<T>{val} {}
    template <typename Integral>
    constexpr RealType(const Rational<Integral> &val) : ArithmeticType<T>{T{val}} {
        if constexpr (std::is_same<T, mpq_class>::value) {
            // mpq_canonicalize(this->value.get_mpq_t());
        }
    }
    constexpr explicit RealType(const ArithmeticType<T> val) : ArithmeticType<T>{val} {}
    RealType &operator=(const ArithmeticType<T> &val)
    {
        this->value = val.value;
        return *this;
    }
};

template <typename T>
struct ProbabilityType : ArithmeticType<T>
{
    constexpr ProbabilityType() : ArithmeticType<T>{} {}
    constexpr ProbabilityType(const T &val) : ArithmeticType<T>{val} {}
    template <typename Integral>
    constexpr ProbabilityType(const Rational<Integral> &val) : ArithmeticType<T>{T{val}} {}
    constexpr explicit ProbabilityType(const ArithmeticType<T> val) : ArithmeticType<T>{val} {
        if constexpr (std::is_same<T, mpq_class>::value) {
            // mpq_canonicalize(this->value.get_mpq_t());
        }
    }
    ProbabilityType &operator=(const ArithmeticType<T> &val)
    {
        this->value = val.value;
        return *this;
    }
};

template <typename T>
struct ObservationType : Wrapper<T>
{
    constexpr ObservationType(const T &value) : Wrapper<T>{value} {}
    constexpr ObservationType() : Wrapper<T>{} {}
    bool operator==(const ObservationType &other) const
    {
        return this->value == other.value;
    }
};

template <typename T>
struct ObservationHashType
{
    std::size_t operator()(const ObservationType<T> &t) const
    {
        return std::hash(static_cast<T>(t));
    }
};

template <>
struct ObservationHashType<std::array<uint8_t, 64>>
{
    std::size_t operator()(const ObservationType<std::array<uint8_t, 64>> &t) const
    {
        const uint64_t *a = reinterpret_cast<const uint64_t *>(static_cast<std::array<uint8_t, 64>>(t).data());
        size_t hash = 0;
        for (int i = 0; i < 8; ++i)
        {
            hash ^= a[i];
        }
        return hash;
    }
};

template <typename T>
struct ActionType : Wrapper<T>
{
    constexpr ActionType(const T &value) : Wrapper<T>{value} {}
    constexpr ActionType() : Wrapper<T>{} {}
};

/*

TypeList

*/

template <
    typename _Real,
    typename _Action,
    typename _Observation,
    typename _Probability,

    template <typename...> typename _Value = PairReal,
    template <typename...> typename _Vector = Vector,
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

using RandomTreeTypes = DefaultTypes<
    double,
    int,
    int,
    double>;

using RatTypes = DefaultTypes<
    mpq_class,
    int,
    int,
    mpq_class,
    ConstantSum<1, 1>::Value>;

template <size_t LogSize>
using BattleTypes = DefaultTypes<
    float,
    uint8_t,
    std::array<uint8_t, LogSize>,
    float>;
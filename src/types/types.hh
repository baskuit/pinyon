#pragma once

#include <libsurskit/math.hh>
#include <types/random.hh>
#include <types/vector.hh>
#include <types/matrix.hh>
#include <types/random.hh>
#include <types/arithmetic.hh>
#include <types/value.hh>
#include <types/wrapper.hh>

#include <vector>

template <typename T>
struct RealType : ArithmeticType<T>
{
    // constexpr RealType(const T val) : ArithmeticType<T>{val} {}
    constexpr RealType(const T &val) : ArithmeticType<T>{val} {}
    constexpr RealType() : ArithmeticType<T>{} {}
    constexpr RealType(const ArithmeticType<T> val) : ArithmeticType<T>{val} {}
    constexpr explicit operator T() const
    {
        return this->value;
    }
    RealType &operator=(const T &val)
    {
        this->value = val;
        return *this;
    }
    RealType &operator=(const ArithmeticType<T> &val)
    {
        this->value = val.value;
        return *this;
    }
};

template <>
struct RealType<mpq_class> : ArithmeticType<mpq_class>
{
    RealType(const mpq_class &val) : ArithmeticType<mpq_class>{val} {}
    RealType() : ArithmeticType<mpq_class>{} {}
    RealType(const ArithmeticType<mpq_class> val) : ArithmeticType<mpq_class>{val} {}
    RealType(const Rational<> &val) : ArithmeticType<mpq_class>{mpq_class{val.p, val.q}} {}
    explicit operator mpq_class () const {
        return this->value;
    }
    RealType &operator=(const mpq_class &val)
    {
        this->value = val;
        return *this;
    }
    RealType &operator=(const ArithmeticType<mpq_class> &val)
    {
        this->value = val.value;
        return *this;
    }
};

template <typename T>
struct ProbabilityType : ArithmeticType<T>
{
    constexpr ProbabilityType(T val) : ArithmeticType<T>{val} {}
    constexpr ProbabilityType() : ArithmeticType<T>{1} {} // prob default init to 1 instead of 0
    constexpr ProbabilityType(const ArithmeticType<T> &val) : ArithmeticType<T>{val} {}
    constexpr ProbabilityType &operator=(const ProbabilityType &other)
    {
        this->value = other.value;
        return *this;
    }
};

template <>
struct ProbabilityType<mpq_class> : ArithmeticType<mpq_class>
{
    ProbabilityType(mpq_class val) : ArithmeticType<mpq_class>{val} {}
    ProbabilityType() : ArithmeticType<mpq_class>{1} {} // prob default init to 1 instead of 0
    ProbabilityType(const ArithmeticType<mpq_class> &val) : ArithmeticType<mpq_class>{val} {}
    ProbabilityType(const Rational<> &val) : ArithmeticType<mpq_class>{mpq_class{val.p, val.q}} {}
    ProbabilityType &operator=(const ProbabilityType &other)
    {
        this->value = other.value;
        return *this;
    }
};

template <typename T>
struct ObservationType : Wrapper<T>
{
    constexpr ObservationType(const T &value) : Wrapper<T>{value} {}
    constexpr ObservationType() : Wrapper<T>{} {}
    constexpr operator T() const
    {
        return Wrapper<T>::value;
    }
    bool operator==(const ObservationType &other) const
    {
        return this->value == other.value;
    }

    // TODO only here for Pokemon logs
    template <typename U, size_t size>
    U *data()
    {
        return this->value.data();
    }
};

template <typename T>
struct _ObservationHash
{
    std::size_t operator()(const ObservationType<T> &t) const
    {
        if constexpr (std::is_same<T, std::array<uint8_t, 64>>::value == true)
        {
            const std::array<uint8_t, 64> x = t;
            const uint64_t *a = reinterpret_cast<const uint64_t *>(x.data());
            size_t hash = 0;
            for (int i = 0; i < 8; ++i)
            {
                hash ^= a[i];
            }
            return hash;
        }
        else
        {
            return std::hash(static_cast<T>(t));
        }
    }
};

template <
    typename _Rational,
    typename _Real,
    typename _Float,

    typename _Action,
    typename _Observation,
    typename _Probability,
    typename _Seed,
    typename _PRNG,
    template <typename _R> typename _VectorReal,
    template <typename _A> typename _VectorAction,
    template <typename _I> typename _VectorInt,
    template <typename _R> typename _MatrixReal,
    template <typename _F> typename _MatrixFloat,
    template <typename _I> typename _MatrixInt>

struct Types
{
    template <typename T>
    struct FloatType : ArithmeticType<T>
    {
        constexpr FloatType(T val) : ArithmeticType<T>{val} {}
        constexpr FloatType() : ArithmeticType<T>{} {}
        constexpr FloatType(const ArithmeticType<T> &val) : ArithmeticType<T>{val} {}
    };

    template <typename T>
    struct ActionType : Wrapper<T>
    {
        constexpr ActionType(const T &value) : Wrapper<T>{value} {}
        constexpr ActionType() : Wrapper<T>{} {}
    };

    template <typename T>
    struct Vector;

    using Rational = _Rational;
    using Real = RealType<_Real>;
    using Float = _Float;

    using Action = ActionType<_Action>;
    using Observation = ObservationType<_Observation>;
    using ObservationHash = _ObservationHash<_Observation>;
    using Probability = ProbabilityType<_Probability>;

    using Seed = _Seed;
    using PRNG = _PRNG;

    using Value = ValueStruct<Real, true>;

    using VectorReal = _VectorReal<Real>;
    using VectorAction = _VectorAction<Action>;
    using VectorInt = _VectorInt<int>;

    using MatrixReal = _MatrixReal<Real>;
    using MatrixFloat = _MatrixFloat<_Float>;
    using MatrixInt = _MatrixInt<int>;
    using MatrixValue = Matrix<Value>;

    using Strategy = VectorReal;
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
    Vector,
    Vector,
    Vector,
    Matrix,
    Matrix,
    Matrix>;

using RandomTreeTypes = Types<
    Rational<int>,
    double,
    double,
    int,
    int,
    double,
    uint64_t,
    prng,
    Vector,
    Vector,
    Vector,
    Matrix,
    Matrix,
    Matrix>;

using RatTypes = Types<
    Rational<int>,
    mpq_class,
    mpq_class,
    int,
    int,
    mpq_class,
    uint64_t,
    prng,
    Vector,
    Vector,
    Vector,
    Matrix,
    Matrix,
    Matrix>;

using ArenaTypes = Types<
    Rational<int>,
    float,
    float,
    uint8_t,
    std::vector<uint8_t>, // train data here TODO
    float,
    uint64_t,
    prng,
    Vector,
    A<9>::Array,
    Vector,
    Matrix,
    Matrix,
    Matrix>;

template <size_t LogSize>
using BattleTypes = Types<
    Rational<int>,
    float,
    float,
    uint8_t,
    std::array<uint8_t, LogSize>,
    float,
    uint64_t,
    prng,
    Vector,
    A<9>::Array,
    Vector,
    Matrix,
    Matrix,
    Matrix>;
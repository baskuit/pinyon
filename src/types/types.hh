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

class EmptyClass
{
public:
    EmptyClass() {}
    ~EmptyClass() {}
    EmptyClass(const EmptyClass &t) {}
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
    typename _MatrixReal,
    typename _MatrixFloat,
    typename _MatrixInt>

struct Types
{

    // template <typename T>
    // struct RationalType : ArithmeticType<T> {
    //     explicit RationalType(T val) : ArithmeticType<T>{val} {}
    //     explicit RationalType () : ArithmeticType<T>{} {}
    // };
    // Rational is basically a primitive, yeah?

    template <typename T>
    struct RealType : ArithmeticType<T>
    {
        // constexpr RealType(const T val) : ArithmeticType<T>{val} {}
        constexpr RealType(const T &val) : ArithmeticType<T>{val} {}
        constexpr RealType() : ArithmeticType<T>{} {}
        constexpr RealType(const ArithmeticType<T> &val) : ArithmeticType<T>{val} {}
        RealType& operator=(const T& val) {
            this->value = RealType(val);
            return *this;
        }
        RealType& operator=(const ArithmeticType<T>& val) {
            this->value = val.value;
            return *this;
        }
    };

    template <typename T>
    struct FloatType : ArithmeticType<T>
    {
        constexpr FloatType(T val) : ArithmeticType<T>{val} {}
        constexpr FloatType() : ArithmeticType<T>{} {}
        constexpr FloatType(const ArithmeticType<T> &val) : ArithmeticType<T>{val} {}
    };
    // These must not be explicit since their operators are defined on Base Class

    template <typename T>
    struct ActionType : Wrapper<T>
    {
        constexpr ActionType(const T &value) : Wrapper<T>{value} {}
        constexpr ActionType() : Wrapper<T>{} {}
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

        // TODO only here for Pokemon logs
        template <typename U, size_t size>
        U* data () {
            return this->value.data();
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

    template <typename T>
    struct Vector;

    using Rational = _Rational;
    using Real = RealType<_Real>;
    using Float = _Float;

    using Action = ActionType<_Action>;
    using Observation = ObservationType<_Observation>;
    using Probability = ProbabilityType<_Probability>;

    using Seed = _Seed;
    using PRNG = _PRNG;

    using Value = ValueStruct<Real, true, 1, 1>;

    using VectorReal = _VectorReal<Real>;
    using VectorAction = _VectorAction<Action>;
    using VectorInt = _VectorInt<int>;

    using MatrixReal = _MatrixReal;
    using MatrixFloat = _MatrixFloat;
    using MatrixInt = _MatrixInt;
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
    Matrix<double>,
    Matrix<double>,
    Matrix<int>>;

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
    Matrix<double>,
    Matrix<double>,
    Matrix<int>>;

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
    Matrix<float>,
    Matrix<float>,
    Matrix<int>>;


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
    Matrix<float>,
    Matrix<float>,
    Matrix<int>>;
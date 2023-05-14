#pragma once

#include <vector>

template <
    typename _Rational,
    typename _Float,
    typename _Action,
    typename _Seed,
    typename _PRNG,
    typename _VectorAction
>
struct Types {

    template <typename T>
    struct FloatType;

    template <typename T>
    struct RationalType {
        T value;

        explicit RationalType(T val) : value(val) {}

        template <typename U>
        explicit operator FloatType<U>() const {
            return FloatType<U>(static_cast<U>(value));
        }
    };

    using Rational = RationalType<_Rational>;

    template <typename T>
    struct FloatType {
        T value;

        explicit FloatType(T val) : value(val) {}
    };

    using Float = FloatType<_Float>;

    template <typename T>
    struct ActionType {
        T value;
        explicit ActionType(T val) : value(val) {}
    };
    using Action = ActionType<_Action>;

    template <typename T>
    struct SeedType {
        T value;
    };
    using Seed = SeedType<_Seed>;

    template <typename T>
    struct PRNGType {
        T value;
    };
    using PRNG = PRNGType<_PRNG>;
};

using SimpleTypes = Types<int, double, char, uint64_t, Empty, std::vector<char>>;
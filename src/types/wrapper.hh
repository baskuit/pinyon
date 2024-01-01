#pragma once

#include <types/rational.hh>

template <typename T>
struct Wrapper
{
    using type = T;
    T value{};
    constexpr Wrapper() {}
    constexpr Wrapper(const T value) : value{value} {}
    constexpr explicit operator T() const { return value; }
    T &get() { return value; }
    const T &get() const { return value; }
};

template <typename T>
struct ArithmeticType : Wrapper<T>
{
    constexpr ArithmeticType() {}

    constexpr ArithmeticType(const T &val) : Wrapper<T>{val} {}

    void canonicalize()
    {
        // if constexpr (requires std::is_same_v<decltype(std::declval<T>().myMethod()), void>) {
        // this->value.canonicalize();

        // }
    }

    template <typename Integral>
    explicit constexpr ArithmeticType(const Rational<Integral> &val) : Wrapper<T>{val} {}

    ArithmeticType operator+(const ArithmeticType &other) const
    {
        return ArithmeticType(this->value + other.value);
    }

    ArithmeticType operator-(const ArithmeticType &other) const
    {
        return ArithmeticType(this->value - other.value);
    }

    ArithmeticType operator*(const ArithmeticType &other) const
    {
        return ArithmeticType(this->value * other.value);
    }

    ArithmeticType operator/(const ArithmeticType &other) const
    {
        return ArithmeticType(this->value / other.value);
    }

    ArithmeticType &operator+=(const ArithmeticType &other)
    {
        this->value += other.value;
        return *this;
    }

    ArithmeticType &operator-=(const ArithmeticType &other)
    {
        this->value -= other.value;
        return *this;
    }

    ArithmeticType &operator*=(const ArithmeticType &other)
    {
        this->value *= other.value;
        return *this;
    }

    ArithmeticType &operator/=(const ArithmeticType &other)
    {
        this->value /= other.value;
        return *this;
    }

    bool operator==(const ArithmeticType &other) const
    {
        if constexpr (std::is_same<T, mpq_class>::value)
        {
            mpq_srcptr a = this->value.get_mpq_t();
            mpq_srcptr b = other.value.get_mpq_t();
            return mpq_equal(a, b) != 0;
        }
        else
        {
            return this->value == other.value;
        }
    }

    bool operator!=(const ArithmeticType &other) const
    {
        if constexpr (std::is_same<T, mpq_class>::value)
        {
            mpq_srcptr a = this->value.get_mpq_t();
            mpq_srcptr b = other.value.get_mpq_t();
            return mpq_equal(a, b) == 0;
        }
        else
        {
            return this->value != other.value;
        }
    }

    bool operator<(const ArithmeticType &other) const
    {
        bool x = this->value < other.value;
        return x;
    }

    bool operator>(const ArithmeticType &other) const
    {
        return this->value > other.value;
    }

    bool operator<=(const ArithmeticType &other) const
    {
        return this->value <= other.value;
    }

    bool operator>=(const ArithmeticType &other) const
    {
        return this->value >= other.value;
    }

    friend std::ostream &operator<<(std::ostream &os, const ArithmeticType &val)
    {
        if constexpr (std::is_same_v<T, mpq_class>)
        {
            os << val.value.get_str();
        }
        else
        {
            os << val.value;
        }
        return os;
    }

    explicit operator double() const
    {
        if constexpr (std::is_same_v<T, mpq_class>)
        {
            return this->value.get_d();
        }
        else
        {
            return static_cast<double>(this->value);
        }
    } // double conversion doesnt work for mpq_class
};

/*

RealType - strongly typed wrapper for the Types::Real class

*/

template <typename T>
struct RealType : ArithmeticType<T>
{
    constexpr RealType() : ArithmeticType<T>{} {}
    constexpr RealType(const T &val) : ArithmeticType<T>{val} {}
    template <typename Integral>
    constexpr RealType(const Rational<Integral> &val) : ArithmeticType<T>{T{val}} {}
    constexpr explicit RealType(const ArithmeticType<T> val) : ArithmeticType<T>{val}
    {
        if constexpr (std::is_same_v<T, mpq_class>)
        {
            mpq_canonicalize(this->value.get_mpq_t());
        }
    }
    RealType &operator=(const ArithmeticType<T> &val)
    {
        this->value = val.value;
        if constexpr (std::is_same_v<T, mpq_class>)
        {
            mpq_canonicalize(this->value.get_mpq_t());
        }
        return *this;
    }

    friend std::ostream &operator<<(std::ostream &os, const RealType &value)
    {
        if constexpr (std::is_same_v<T, mpq_class>)
        {
            os << value.value.get_str();
        }
        else
        {
            os << value.value;
        }
        return os;
    }
};

template <typename T>
struct ProbType : ArithmeticType<T>
{
    constexpr ProbType() : ArithmeticType<T>{} {}
    constexpr ProbType(const T &val) : ArithmeticType<T>{val}
    {
        if constexpr (std::is_same_v<T, mpq_class>)
        {
        }
    }
    template <typename Integral>
    constexpr ProbType(const Rational<Integral> &val) : ArithmeticType<T>{T{val}}
    {
        if constexpr (std::is_same_v<T, mpq_class>)
        {
        }
    }
    constexpr explicit ProbType(const ArithmeticType<T> val) : ArithmeticType<T>{val}
    {
        if constexpr (std::is_same_v<T, mpq_class>)
        {
            mpq_canonicalize(this->value.get_mpq_t());
        }
    }
    ProbType &operator=(const ArithmeticType<T> &val)
    {
        this->value = val.value;
        if constexpr (std::is_same_v<T, mpq_class>)
        {
        }
        return *this;
    }
};

template <typename T>
struct ObsType : Wrapper<T>
{
    constexpr ObsType(const T &value) : Wrapper<T>{value} {}
    constexpr ObsType() : Wrapper<T>{} {}
    bool operator==(const ObsType &other) const
    {
        return this->value == other.value;
    }
};

template <typename T>
struct ObsHashType
{
    size_t operator()(const ObsType<T> &obs) const
    {
        return static_cast<T>(obs); // TODO
    }
};

template <>
struct ObsHashType<std::array<uint8_t, 64>>
{
    size_t operator()(const ObsType<std::array<uint8_t, 64>> &obs) const
    {
        const uint64_t *a = reinterpret_cast<const uint64_t *>(obs.value.data());
        size_t hash = 0;
        for (int i = 0; i < 8; ++i)
        {
            hash ^= a[i];
        }
        return hash;
    }
};
size_t hash_pair(size_t a, size_t b) {
    size_t seed = 0;  // Initial seed value
    constexpr size_t m = (size_t)-58;  // Large prime number

    // Mix the bits of the first value
    seed ^= a + m + (seed << 6) + (seed >> 2);

    // Mix the bits of the second value
    seed ^= b + m + (seed << 6) + (seed >> 2);

    // Final mixing to ensure good distribution
    seed += (seed << 3);
    seed ^= (seed >> 11);
    seed += (seed << 15);

    return seed;
}
template <>
struct ObsHashType<std::array<uint8_t, 16>>
{
    size_t xs (size_t state) const
    {
        state ^= (state << 21);
        state ^= (state >> 35);
        state ^= (state << 4);
        return state;
    }
    size_t operator()(const ObsType<std::array<uint8_t, 16>> &obs) const
    {
        const uint64_t *a = reinterpret_cast<const uint64_t *>(obs.value.data());
        // return hash_pair(a[0], a[1]);
        return ((a[0] << 32) >> 32) | (a[1] << 32);
    }
};

template <>
struct ObsHashType<std::array<uint8_t, 376>>
{

    size_t operator()(const ObsType<std::array<uint8_t, 376>> &obs) const
    {
        const uint64_t *a = reinterpret_cast<const uint64_t *>(obs.value.data());
        size_t hash = 0;
        for (int i = 0; i < 47; ++i)
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
    ActionType &operator=(const T &value)
    {
        this->value = value;
        return *this;
    }
    bool operator==(const ActionType &value) const
    {
        return this->value == value.value;
    }
};

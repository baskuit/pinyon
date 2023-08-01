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
        os << val.value;
        return os;
    }
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
struct ProbabilityType : ArithmeticType<T>
{
    constexpr ProbabilityType() : ArithmeticType<T>{} {}
    constexpr ProbabilityType(const T &val) : ArithmeticType<T>{val}
    {
        if constexpr (std::is_same_v<T, mpq_class>)
        {
        }
    }
    template <typename Integral>
    constexpr ProbabilityType(const Rational<Integral> &val) : ArithmeticType<T>{T{val}}
    {
        if constexpr (std::is_same_v<T, mpq_class>)
        {
        }
    }
    constexpr explicit ProbabilityType(const ArithmeticType<T> val) : ArithmeticType<T>{val}
    {
        if constexpr (std::is_same_v<T, mpq_class>)
        {
            mpq_canonicalize(this->value.get_mpq_t());
        }
    }
    ProbabilityType &operator=(const ArithmeticType<T> &val)
    {
        this->value = val.value;
        if constexpr (std::is_same_v<T, mpq_class>)
        {
        }
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
    size_t operator()(const ObservationType<T> &t) const
    {
        return std::hash(static_cast<T>(t));
    }
};

template <>
struct ObservationHashType<std::array<uint8_t, 64>>
{
    size_t operator()(const ObservationType<std::array<uint8_t, 64>> &t) const
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
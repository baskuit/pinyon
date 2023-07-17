#pragma once

template <typename T = int>
struct Rational
{
    T p = 1;
    T q = 1;
    operator mpq_class() const
    {
        return mpq_class{p, q};
    }
};

template <typename T>
struct Wrapper
{
    using type = T;
    T value{};
    constexpr Wrapper() {}
    constexpr Wrapper(const T value) : value{value} {}
    constexpr explicit operator T() const { return value; }
    constexpr Wrapper operator=(const T &value)
    {
        this->value = value;
        return *this;
    }
};

template <>
struct Wrapper<mpq_class>
{
    using type = mpq_class;
    mpq_class value{};
    Wrapper() {}
    Wrapper(const mpq_class &value) : value{value} {}
    Wrapper(const Rational<> &value) : value{value.p, value.q} {}
    explicit operator mpq_class() const { return value; }
    Wrapper<mpq_class> operator=(const mpq_class &value)
    {
        this->value = value;
        return *this;
    }
};

template <typename T>
struct ArithmeticType : Wrapper<T>
{
    constexpr ArithmeticType() {}
    explicit constexpr ArithmeticType(const T &val) : Wrapper<T>{val} {}
    constexpr ArithmeticType(const Wrapper<T> &val) : Wrapper<T>{val} {}
};

template <typename T>
struct RealType : ArithmeticType<T>
{
    constexpr RealType() : ArithmeticType<T>{} {}
    constexpr RealType(const Wrapper<T> &val) : Wrapper<T>{val} {}
    constexpr explicit RealType(const ArithmeticType<T> val) : ArithmeticType<T>{val} {}
    RealType &operator=(const Wrapper<T> &val)
    {
        this->value = val.value;
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
    RealType() : ArithmeticType<mpq_class>{} {}
    RealType(const Wrapper<mpq_class> &val) : ArithmeticType<mpq_class>{val}
    {
    }
    explicit RealType(const ArithmeticType<mpq_class> &val) : ArithmeticType<mpq_class>{val}
    {
    }
    explicit operator mpq_class() const
    {
        return this->value;
    }
    explicit operator double() const
    {
        return this->value.get_d();
    }
};

template <typename T>
struct ProbabilityType : ArithmeticType<T>
{
    constexpr ProbabilityType() : ArithmeticType<T>{} {}
    constexpr ProbabilityType(const Wrapper<T> &val) : ArithmeticType<T>{val} {}
    constexpr explicit ProbabilityType(const ArithmeticType<T> &val) : ArithmeticType<T>{val} {}
};

template <>
struct ProbabilityType<mpq_class> : ArithmeticType<mpq_class>
{
    ProbabilityType() : ArithmeticType<mpq_class>{} {}
    ProbabilityType(const mpq_class &val) : ArithmeticType<mpq_class>{val}
    {
    }
    ProbabilityType(const ArithmeticType<mpq_class> &val) : ArithmeticType<mpq_class>{val}
    {
    }
    explicit operator mpq_class() const
    {
        return this->value;
    }
    explicit operator double() const
    {
        return this->value.get_d();
    }
};
#include <types/rational.hh>

template <typename T>
struct ArithmeticType : Wrapper<T>
{
    constexpr ArithmeticType() {}

    constexpr ArithmeticType(const T &val) : Wrapper<T>{val} {}

    // constexpr ArithmeticType(const ArithmeticType &val) : Wrapper<T>{val.value}
    // {
    //     if constexpr (std::is_same_v<T, mpq_class>)
    //     {
    //         mpq_canonicalize(this->value.get_mpq_t());
    //     }
    // }

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

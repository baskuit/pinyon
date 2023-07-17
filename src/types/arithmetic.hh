#include <types/rational.hh>

template <typename T>
struct ArithmeticType : Wrapper<T>
{
    constexpr ArithmeticType() {}

    explicit constexpr ArithmeticType(const T &val) : Wrapper<T>{val} {}
    // mostly to limit its use to this class

    constexpr ArithmeticType(const Wrapper<T> &val) : Wrapper<T>{val} {}

    // explicit operator Wrapper<T>() {
    //     return *this;
    // }

    // ArithmeticType &operator=(const T &t)
    // {
    //     this->value = t;
    //     return *this;
    // }

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
            return this->value == other;
        }
    }

    bool operator!=(const ArithmeticType &other) const
    {
        return this->value != other.value;
    }

    bool operator<(const ArithmeticType &other) const
    {
        return this->value < other.value;
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

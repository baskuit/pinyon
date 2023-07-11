#include <types/rational.hh>

template <typename T>
struct ArithmeticType : Wrapper<T>
{
    constexpr ArithmeticType() {}

    constexpr ArithmeticType(const T val) : Wrapper<T>{val} {}

    constexpr explicit operator T() const
    {
        return this->value;
    }

    constexpr T unwrap() const
    {
        return this->value;
    }

    ArithmeticType &operator=(const T &t)
    {
        this->value = t;
        return (*this);
    }

    template <typename U>
    ArithmeticType &operator=(const Rational<T> &t)
    {
        this->value = t;
        return (*this);
    }

    ArithmeticType operator+(const ArithmeticType &other) const
    {
        return ArithmeticType(this->value + other.value);
    }

    ArithmeticType operator+(const T &other) const
    {
        return ArithmeticType(this->value + other);
    }

    template <typename U>
    ArithmeticType operator+(const Rational<U> &other) const
    {
        return ArithmeticType(this->value + other);
    }

    ArithmeticType operator-(const ArithmeticType &other) const
    {
        return ArithmeticType(this->value - other.value);
    }

    ArithmeticType operator-(const T &other) const
    {
        return ArithmeticType(this->value - other);
    }

    template <typename U>
    ArithmeticType operator-(const Rational<U> &other) const
    {
        return ArithmeticType(this->value - other);
    }

    ArithmeticType operator*(const ArithmeticType &other) const
    {
        return ArithmeticType(this->value * other.value);
    }

    ArithmeticType operator*(const T &other) const
    {
        return ArithmeticType(this->value * other);
    }

    template <typename U>
    ArithmeticType operator*(const Rational<U> &other) const
    {
        return ArithmeticType(this->value * other);
    }

    ArithmeticType operator/(const ArithmeticType &other) const
    {
        return ArithmeticType(this->value / other.value);
    }

    ArithmeticType operator/(const T &other) const
    {
        return ArithmeticType(this->value / other);
    }

    template <typename U>
    ArithmeticType operator/(const Rational<U> &other) const
    {
        return ArithmeticType(this->value / other);
    }

    ArithmeticType &operator+=(const ArithmeticType &other)
    {
        this->value += other.value;
        return *this;
    }

    ArithmeticType &operator+=(const T &other)
    {
        this->value += other;
        return *this;
    }

    template <typename U>
    ArithmeticType &operator+=(const Rational<U> &other)
    {
        this->value += other;
        return *this;
    }

    ArithmeticType &operator-=(const ArithmeticType &other)
    {
        this->value -= other.value;
        return *this;
    }

    ArithmeticType &operator-=(const T &other)
    {
        this->value -= other;
        return *this;
    }

    template <typename U>
    ArithmeticType &operator-=(const Rational<T> &other)
    {
        this->value -= other;
        return *this;
    }

    ArithmeticType &operator*=(const ArithmeticType &other)
    {
        this->value *= other.value;
        return *this;
    }

    ArithmeticType &operator*=(const T &other)
    {
        this->value *= other;
        return *this;
    }

    template <typename U>
    ArithmeticType &operator*=(const Rational<U> &other)
    {
        this->value *= other;
        return *this;
    }

    ArithmeticType &operator/=(const ArithmeticType &other)
    {
        this->value /= other.value;
        return *this;
    }

    ArithmeticType &operator/=(const T &other)
    {
        this->value /= other;
        return *this;
    }

    template <typename U>
    ArithmeticType &operator/=(const Rational<U> &other)
    {
        this->value /= other;
        return *this;
    }

    bool operator==(const ArithmeticType &other) const
    {
        return this->value == other.value;
    }

    bool operator==(const T &other) const
    {
        return this->value == other;
    }

    template <typename U>
    bool operator==(const Rational<U> &other) const
    {
        return this->value == other;
    }

    bool operator!=(const ArithmeticType &other) const
    {
        return this->value != other.value;
    }

    bool operator!=(const T &other) const
    {
        return this->value != other;
    }

    template <typename U>
    bool operator!=(const Rational<U> &other) const
    {
        return this->value != other;
    }

    bool operator<(const ArithmeticType &other) const
    {
        return this->value < other.value;
    }

    bool operator<(const T &other) const
    {
        return this->value < other;
    }

    template <typename U>
    bool operator<(const Rational<U> &other) const
    {
        return this->value < other;
    }

    bool operator>(const ArithmeticType &other) const
    {
        return this->value > other.value;
    }

    bool operator>(const T &other) const
    {
        return this->value > other;
    }

    template <typename U>
    bool operator>(const Rational<U> &other) const
    {
        return this->value > other;
    }

    bool operator<=(const ArithmeticType &other) const
    {
        return this->value <= other.value;
    }

    bool operator<=(const T &other) const
    {
        return this->value <= other;
    }

    template <typename U>
    bool operator<=(const Rational<U> &other) const
    {
        return this->value <= other;
    }

    bool operator>=(const ArithmeticType &other) const
    {
        return this->value >= other.value;
    }

    bool operator>=(const T &other) const
    {
        return this->value >= other;
    }

    template <typename U>
    bool operator>=(const Rational<U> &other) const
    {
        return this->value >= other;
    }

    friend std::ostream &operator<<(std::ostream &os, const ArithmeticType &val)
    {
        os << val.value;
        return os;
    }
};

template <typename T>
struct ArithmeticType
{
    T value;

    explicit ArithmeticType() {}

    explicit ArithmeticType(const T &val) : value(val) {}

    operator T() const
    {
        return value;
    }

    ArithmeticType &operator=(const T &t)
    {
        value = t;
        return (*this);
    }

    template <typename U>
    ArithmeticType &operator=(const Rational<T> &t)
    {
        value = t;
        return (*this);
    }

    ArithmeticType operator+(const ArithmeticType &other) const
    {
        return ArithmeticType(value + other.value);
    }

    ArithmeticType operator+(const T &other) const
    {
        return ArithmeticType(value + other);
    }

    template <typename U>
    ArithmeticType operator+(const Rational<U> &other) const
    {
        return ArithmeticType(value + other);
    }

    ArithmeticType operator-(const ArithmeticType &other) const
    {
        return ArithmeticType(value - other.value);
    }

    ArithmeticType operator-(const T &other) const
    {
        return ArithmeticType(value - other);
    }

    template <typename U>
    ArithmeticType operator-(const Rational<U> &other) const
    {
        return ArithmeticType(value - other);
    }

    ArithmeticType operator*(const ArithmeticType &other) const
    {
        return ArithmeticType(value * other.value);
    }

    ArithmeticType operator*(const T &other) const
    {
        return ArithmeticType(value * other);
    }

    template <typename U>
    ArithmeticType operator*(const Rational<U> &other) const
    {
        return ArithmeticType(value * other);
    }

    ArithmeticType operator/(const ArithmeticType &other) const
    {
        return ArithmeticType(value / other.value);
    }

    ArithmeticType operator/(const T &other) const
    {
        return ArithmeticType(value / other);
    }

    template <typename U>
    ArithmeticType operator/(const Rational<U> &other) const
    {
        return ArithmeticType(value / other);
    }

    ArithmeticType &operator+=(const ArithmeticType &other)
    {
        value += other.value;
        return *this;
    }

    ArithmeticType &operator+=(const T &other)
    {
        value += other;
        return *this;
    }

    template <typename U>
    ArithmeticType &operator+=(const Rational<U> &other)
    {
        value += other;
        return *this;
    }

    ArithmeticType &operator-=(const ArithmeticType &other)
    {
        value -= other.value;
        return *this;
    }

    ArithmeticType &operator-=(const T &other)
    {
        value -= other;
        return *this;
    }

    template <typename U>
    ArithmeticType &operator-=(const Rational<T> &other)
    {
        value -= other;
        return *this;
    }

    ArithmeticType &operator*=(const ArithmeticType &other)
    {
        value *= other.value;
        return *this;
    }

    ArithmeticType &operator*=(const T &other)
    {
        value *= other;
        return *this;
    }

    template <typename U>
    ArithmeticType &operator*=(const Rational<U> &other)
    {
        value *= other;
        return *this;
    }

    ArithmeticType &operator/=(const ArithmeticType &other)
    {
        value /= other.value;
        return *this;
    }

    ArithmeticType &operator/=(const T &other)
    {
        value /= other;
        return *this;
    }

    template <typename U>
    ArithmeticType &operator/=(const Rational<U> &other)
    {
        value /= other;
        return *this;
    }

    bool operator==(const ArithmeticType &other) const
    {
        return value == other.value;
    }

    bool operator==(const T &other) const
    {
        return value == other;
    }

    template <typename U>
    bool operator==(const Rational<U> &other) const
    {
        return value == other;
    }

    bool operator!=(const ArithmeticType &other) const
    {
        return value != other.value;
    }

    bool operator!=(const T &other) const
    {
        return value != other;
    }

    template <typename U>
    bool operator!=(const Rational<U> &other) const
    {
        return value != other;
    }

    bool operator<(const ArithmeticType &other) const
    {
        return value < other.value;
    }

    bool operator<(const T &other) const
    {
        return value < other;
    }

    template <typename U>
    bool operator<(const Rational<U> &other) const
    {
        return value < other;
    }

    bool operator>(const ArithmeticType &other) const
    {
        return value > other.value;
    }

    bool operator>(const T &other) const
    {
        return value > other;
    }

    template <typename U>
    bool operator>(const Rational<U> &other) const
    {
        return value > other;
    }

    bool operator<=(const ArithmeticType &other) const
    {
        return value <= other.value;
    }

    bool operator<=(const T &other) const
    {
        return value <= other;
    }

    template <typename U>
    bool operator<=(const Rational<U> &other) const
    {
        return value <= other;
    }

    bool operator>=(const ArithmeticType &other) const
    {
        return value >= other.value;
    }

    bool operator>=(const T &other) const
    {
        return value >= other;
    }

    template <typename U>
    bool operator>=(const Rational<U> &other) const
    {
        return value >= other;
    }
};
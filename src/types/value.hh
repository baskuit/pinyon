#pragma once

#include <gmpxx.h>

template <int PAYOFF_SUM_NUM, int PAYOFF_SUM_DEN>
struct ConstantSum
{
    template <typename Real>
    struct Value
    {
        static constexpr bool IS_CONSTANT_SUM{true};
        static constexpr Rational<> PAYOFF_SUM{PAYOFF_SUM_NUM, PAYOFF_SUM_DEN};

        Real row_value{Rational<>{0}};

        Value() {}
        Value(const Real row_value) : row_value{row_value} {}

        inline constexpr Real get_row_value() const
        {
            return row_value;
        }
        inline constexpr Real get_col_value() const
        {
            return Real{PAYOFF_SUM} - row_value;
        }

        template <typename T>
        Value &operator+=(const T t)
        {
            row_value += static_cast<Real>(t);
            return *this;
        }

        Value &operator+=(const Value t)
        {
            row_value += static_cast<Real>(t.row_value);
            return *this;
        }

        template <typename T>
        Value operator*(const T t)
        {
            return Value{Real{row_value * static_cast<Real>(t)}};
        }

        friend std::ostream &operator<<(std::ostream &os, const Value &x)
        {
            os << x.row_value;
            return os;
        }
    };
};

template <typename Real>
struct PairReal
{
    static constexpr bool IS_CONSTANT_SUM{false};

    Real row_value{Rational<>{0}};
    Real col_value{Rational<>{0}};

    PairReal() {}
    PairReal(Real row_value, Real col_value) : row_value{row_value}, col_value{col_value} {}

    inline constexpr Real get_row_value() const
    {
        return row_value;
    }
    inline constexpr Real get_col_value() const
    {
        return col_value;
    }

    PairReal &operator+=(const PairReal &other)
    {
        row_value += other.row_value;
        col_value += other.col_value;
        return *this;
    }

    constexpr PairReal operator+(const PairReal other) const
    {
        return PairReal{row_value + other.row_value, col_value + other.col_value};
    }

    constexpr PairReal operator*(const Real val) const
    {
        return PairReal{row_value * val, col_value * val};
    }

    friend std::ostream &operator<<(std::ostream &os, const PairReal &value)
    {
        os << value.row_value << ',' << value.col_value;
        return os;
    }
};

template <template <typename> typename Wrapper>
struct PairReal<Wrapper<mpq_class>>
{
    static constexpr bool IS_CONSTANT_SUM{false};

    mpq_class x{};
    Wrapper<int> y;
    Wrapper<mpq_class> row_value{Rational<>{0}};
    Wrapper<mpq_class> col_value{Rational<>{0}};

    PairReal() {}
    PairReal(mpq_class row_value, mpq_class col_value) : row_value{row_value}, col_value{col_value} {}
    // template <typename T>
    // PairReal(Rational<T> row_value, Rational<T> col_value) : row_value{row_value}, col_value{col_value} {}

    inline constexpr Wrapper<mpq_class> get_row_value() const
    {
        return row_value;
    }
    inline constexpr Wrapper<mpq_class> get_col_value() const
    {
        return col_value;
    }

    PairReal<Wrapper<mpq_class>> &operator+=(const PairReal<Wrapper<mpq_class>> &other)
    {
        row_value += other.row_value;
        col_value += other.col_value;
        return *this;
    }

    PairReal<Wrapper<mpq_class>> operator+(const PairReal<Wrapper<mpq_class>> other) const
    {
        return PairReal<Wrapper<mpq_class>>{row_value + other.row_value, col_value + other.col_value};
    }

    PairReal<Wrapper<mpq_class>> operator*(const Wrapper<mpq_class> val) const
    {
        return PairReal<Wrapper<mpq_class>>{static_cast<mpq_class>(row_value * val), static_cast<mpq_class>(col_value * val)};
    }

    friend std::ostream &operator<<(std::ostream &os, const PairReal<Wrapper<mpq_class>> &value)
    {
        os << value.row_value.value.get_d() << ',' << value.col_value.value.get_d();
        return os;
    }
};

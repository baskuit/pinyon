#pragma once

#include <gmpxx.h>

template <typename Real, bool _IS_CONSTANT_SUM, int PAYOFF_SUM_NUM = 1, int PAYOFF_SUM_DEN = 1>
struct ValueStruct
{
    static constexpr bool IS_CONSTANT_SUM{_IS_CONSTANT_SUM};
    static constexpr Real PAYOFF_SUM{Rational{PAYOFF_SUM_NUM, PAYOFF_SUM_DEN}};
    Real row_value{Rational{0}};
    Real col_value{Rational{0}};

    ValueStruct() {}
    ValueStruct(Real row_value, Real col_value) : row_value{row_value}, col_value{col_value} {}

    template <typename T>
    operator ValueStruct<T, _IS_CONSTANT_SUM, PAYOFF_SUM_NUM, PAYOFF_SUM_DEN>() const
    {
        return ValueStruct<T, _IS_CONSTANT_SUM, PAYOFF_SUM_NUM, PAYOFF_SUM_DEN>{static_cast<T>(row_value), static_cast<T>(col_value)};
    }

    inline Real get_row_value() const
    {
        return row_value;
    }
    inline Real get_col_value() const
    {
        return col_value;
    }
    ValueStruct &operator+=(const ValueStruct &other)
    {
        row_value += other.row_value;
        col_value += other.col_value;
        return *this;
    }

    ValueStruct operator+(const ValueStruct other)
    {
        return ValueStruct{row_value + other.row_value, col_value + other.col_value};
    }

    ValueStruct operator*(const Real val)
    {
        return ValueStruct{row_value * val, col_value * val};
    }

    friend std::ostream &operator<<(std::ostream &os, const ValueStruct &session)
    {
        os << session.row_value;
        return os;
    }
};

template <typename Real, int PAYOFF_SUM_NUM, int PAYOFF_SUM_DEN>
struct ValueStruct<Real, true, PAYOFF_SUM_NUM, PAYOFF_SUM_DEN>
{
    static constexpr bool IS_CONSTANT_SUM{true};
    static constexpr Rational<> PAYOFF_SUM{PAYOFF_SUM_NUM, PAYOFF_SUM_DEN};
    Real row_value{Rational{0}};

    ValueStruct() {}
    ValueStruct(const Real row_value) : row_value{row_value} {}
    ValueStruct(const Real row_value, const Real col_value) : row_value{row_value} {}

    inline Real get_row_value() const
    {
        return row_value;
    }
    inline Real get_col_value() const
    {
        return Real{PAYOFF_SUM} - row_value;
    }
    ValueStruct &operator+=(const ValueStruct &other)
    {
        row_value += other.row_value;
        return *this;
    }

    ValueStruct operator*(const Real val)
    {
        return ValueStruct{row_value * val};
    }

    friend std::ostream &operator<<(std::ostream &os, const ValueStruct &session)
    {
        os << session.row_value;
        return os;
    }
};

struct PairRational
{

    mpq_class row_value{};
    mpq_class col_value{};

    PairRational() {}
    PairRational(const mpq_class row_value) : row_value{row_value}, col_value{1 - row_value} {}
    PairRational(const mpq_class row_value, const mpq_class col_value) : row_value{row_value} {}

    inline mpq_class get_row_value() const
    {
        return row_value;
    }
    inline mpq_class get_col_value() const
    {
        return col_value;
    }

    friend std::ostream &operator<<(std::ostream &os, const PairRational &session)
    {
        os << session.row_value;
        return os;
    }
};

using PairDouble = ValueStruct<double, false, 0, 1>;
#pragma once

/*

Exp3 is faster rn without assuming constant sum because then it saves the col_value, instead of recomputing each node lol

*/

template <typename Real, bool IS_CONSTANT_SUM, int PAYOFF_SUM_NUM=1, int PAYOFF_SUM_DEN=1>
struct Value {
    static constexpr Real PAYOFF_SUM{Rational{PAYOFF_SUM_NUM, PAYOFF_SUM_DEN}};
    Real row_value;
    Real col_value;
    Value () {}
    Value(Real row_value, Real col_value) : row_value{row_value}, col_value{col_value} {}
    inline Real get_row_value () {
        return row_value;
    }
    inline Real get_col_value () {
        return col_value;
    }
    Value& operator+=(const Value& other) {
        row_value += other.row_value;
        col_value += other.col_value;
        return *this;
    }
};

template <typename Real, int PAYOFF_SUM_NUM, int PAYOFF_SUM_DEN>
struct Value<Real, true, PAYOFF_SUM_NUM, PAYOFF_SUM_DEN> {
    static constexpr Real PAYOFF_SUM{Rational{PAYOFF_SUM_NUM, PAYOFF_SUM_DEN}};
    Real row_value;
    Value () {}
    Value(Real row_value, Real col_value) : row_value{row_value} {}
    inline Real get_row_value () {
        return row_value;
    }
    inline Real get_col_value () {
        return PAYOFF_SUM - row_value;
    }
    Value& operator+=(const Value& other) {
        row_value += other.row_value;
        return *this;
    }
};
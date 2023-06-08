#pragma once

/*

Exp3 is faster rn without assuming constant sum because then it saves the col_value, instead of recomputing each node lol

*/

template <typename Real, bool IS_CONSTANT_SUM, int PAYOFF_SUM_NUM=1, int PAYOFF_SUM_DEN=1>
struct ValueStruct {
    static constexpr Real PAYOFF_SUM{Rational{PAYOFF_SUM_NUM, PAYOFF_SUM_DEN}};
    Real row_value;
    Real col_value;
    ValueStruct () {}
    ValueStruct(Real row_value, Real col_value) : row_value{row_value}, col_value{col_value} {}
    inline Real get_row_value () const {
        return row_value;
    }
    inline Real get_col_value () const {
        return col_value;
    }
    ValueStruct& operator+=(const ValueStruct& other) {
        row_value += other.row_value;
        col_value += other.col_value;
        return *this;
    }

    template <typename T>
    ValueStruct operator*(T val) {
        return ValueStruct{row_value * val, col_value * val};
    }
};

template <typename Real, int PAYOFF_SUM_NUM, int PAYOFF_SUM_DEN>
struct ValueStruct<Real, true, PAYOFF_SUM_NUM, PAYOFF_SUM_DEN> {
    static constexpr Real PAYOFF_SUM{Rational{PAYOFF_SUM_NUM, PAYOFF_SUM_DEN}};
    Real row_value;
    ValueStruct () {}
    ValueStruct (Real row_value) : row_value{row_value} {}
    ValueStruct(Real row_value, Real col_value) : row_value{row_value} {}
    inline Real get_row_value () const {
        return row_value;
    }
    inline Real get_col_value () const {
        return PAYOFF_SUM - row_value;
    }
    ValueStruct& operator+=(const ValueStruct& other) {
        row_value += other.row_value;
        return *this;
    }

    template <typename T>
    ValueStruct operator*(T val) {
        return ValueStruct{row_value * val};
    }
};
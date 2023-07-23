#pragma once

#include "../../extern/lrslib/include/lib.h"

#include <types/types.hh>

namespace LRSNash
{

    // template <typename Real, template <typename> typename Vector, template <typename> typename Matrix>
    // void solve(Matrix<Real> &payoff_matrix, Vector<Real> &row_strategy, Vector<Real> &col_strategy)
    // {
    // }

    template <template <typename...> typename Vector, template <typename...> typename Matrix, template <typename> typename Wrapper>
    std::pair<Wrapper<mpq_class>, Wrapper<mpq_class>>
    solve(
        Matrix<PairReal<Wrapper<mpq_class>>> &payoff_matrix,
        Vector<Wrapper<mpq_class>> &row_strategy,
        Vector<Wrapper<mpq_class>> &col_strategy)
    {
        const size_t rows = payoff_matrix.rows;
        const size_t cols = payoff_matrix.cols;
        const size_t entries = rows * cols;
        std::vector<mpq_t *> rpd{entries}, cpd{entries};

        for (size_t i = 0; i < entries; ++i)
        {
            rpd[i] = reinterpret_cast<mpq_t *>(&payoff_matrix[i].row_value);
            cpd[i] = reinterpret_cast<mpq_t *>(&payoff_matrix[i].col_value);
        }

        mpz_t *row_solution_data = new mpz_t[rows + 2];
        mpz_t *col_solution_data = new mpz_t[cols + 2];
        for (int row_idx = 0; row_idx < rows + 2; ++row_idx)
        {
            mpz_init(row_solution_data[row_idx]);
        }
        for (int col_idx = 0; col_idx < cols + 2; ++col_idx)
        {
            mpz_init(col_solution_data[col_idx]);
        }

        solve_gmp_pointer(rows, cols, rpd.data(), cpd.data(), row_solution_data, col_solution_data);

        mpz_class row_den{row_solution_data[0]}, col_den{col_solution_data[0]};
        for (int row_idx = 0; row_idx < rows; ++row_idx)
        {
            row_strategy[row_idx] = Wrapper<mpq_class>{mpq_class{mpz_class{row_solution_data[row_idx + 1]}, row_den}};
        }
        for (int col_idx = 0; col_idx < cols; ++col_idx)
        {
            col_strategy[col_idx] = Wrapper<mpq_class>{mpq_class{mpz_class{col_solution_data[col_idx + 1]}, col_den}};
        }

        mpq_class row_payoff{mpz_class{col_solution_data[cols + 1]}, col_den};
        mpq_class col_payoff{mpz_class{row_solution_data[rows + 1]}, row_den};

        delete[] row_solution_data;
        delete[] col_solution_data;

        return {Wrapper<mpq_class>{row_payoff}, Wrapper<mpq_class>{col_payoff}};
    }

    template <template <typename...> typename Vector, template <typename...> typename Matrix, template <typename> typename Wrapper>
    std::pair<Wrapper<mpq_class>, Wrapper<mpq_class>>
    solve(
        Matrix<ConstantSum<1, 1>::Value<Wrapper<mpq_class>>> &payoff_matrix,
        Vector<Wrapper<mpq_class>> &row_strategy,
        Vector<Wrapper<mpq_class>> &col_strategy)
    {
        const size_t rows = payoff_matrix.rows;
        const size_t cols = payoff_matrix.cols;
        const size_t entries = rows * cols;
        std::vector<mpq_t *> rpd{entries};

        for (size_t i = 0; i < entries; ++i)
        {
            rpd[i] = reinterpret_cast<mpq_t *>(&payoff_matrix[i].row_value);
        }

        mpz_t *row_solution_data = new mpz_t[rows + 2];
        mpz_t *col_solution_data = new mpz_t[cols + 2];
        for (int row_idx = 0; row_idx < rows + 2; ++row_idx)
        {
            mpz_init(row_solution_data[row_idx]);
        }
        for (int col_idx = 0; col_idx < cols + 2; ++col_idx)
        {
            mpz_init(col_solution_data[col_idx]);
        }

        solve_gmp_pointer_constant_sum(rows, cols, rpd.data(), row_solution_data, col_solution_data, 1, 1);

        mpz_class row_den{row_solution_data[0]}, col_den{col_solution_data[0]};
        row_strategy.fill(rows);
        col_strategy.fill(cols);
        for (int row_idx = 0; row_idx < rows; ++row_idx)
        {
            row_strategy[row_idx] = Wrapper<mpq_class>{mpq_class{mpz_class{row_solution_data[row_idx + 1]}, row_den}};
        }
        for (int col_idx = 0; col_idx < cols; ++col_idx)
        {
            col_strategy[col_idx] = Wrapper<mpq_class>{mpq_class{mpz_class{col_solution_data[col_idx + 1]}, col_den}};
        }

        mpq_class row_payoff{mpz_class{col_solution_data[cols + 1]}, col_den};
        mpq_class col_payoff{mpz_class{row_solution_data[rows + 1]}, row_den};

        delete[] row_solution_data;
        delete[] col_solution_data;

        return {Wrapper<mpq_class>{row_payoff}, Wrapper<mpq_class>{col_payoff}};
    }

    template <template <typename...> typename Vector, template <typename...> typename Matrix, template <typename> typename Value, typename Real>
    std::pair<Real, Real>
    solve(
        Matrix<Value<Real>> &payoff_matrix,
        Vector<Real> &row_strategy,
        Vector<Real> &col_strategy,
        long den = 100)
    {
        const size_t rows = payoff_matrix.rows;
        const size_t cols = payoff_matrix.cols;
        const size_t entries = rows * cols;

        const Real min = payoff_matrix.min();
        const Real max = payoff_matrix.max();
        const Real range{max == min ? 1 : max - min};

        std::vector<long> payoff_data;
        payoff_data.resize(2 * entries);

        // payoff data is just double length array of row_num and col_den in discretization

        for (size_t i = 0; i < entries; ++i)
        {
            Value<Real> &value = payoff_matrix[i];
            double a{(value.get_row_value() - min) / range * den};
            double b{(value.get_col_value() - min) / range * den};
            payoff_data[2 * i] = ceil(a);
            payoff_data[2 * i + 1] = ceil(b);
        }

        mpz_t *row_solution_data = new mpz_t[rows + 2];
        mpz_t *col_solution_data = new mpz_t[cols + 2];
        for (int row_idx = 0; row_idx < rows + 2; ++row_idx)
        {
            mpz_init(row_solution_data[row_idx]);
        }
        for (int col_idx = 0; col_idx < cols + 2; ++col_idx)
        {
            mpz_init(col_solution_data[col_idx]);
        }

        solve_gmp_float(rows, cols, payoff_data.data(), den, row_solution_data, col_solution_data);

        mpz_class row_den{row_solution_data[0]}, col_den{col_solution_data[0]};
        row_strategy.fill(rows);
        col_strategy.fill(cols);
        for (int row_idx = 0; row_idx < rows; ++row_idx)
        {
            // row_strategy[row_idx] = Real{mpz_get_ui(row_solution_data[row_idx + 1]) / static_cast<typename Real::type>(row_den)};
            row_strategy[row_idx] = Real{mpq_class{mpz_class{row_solution_data[row_idx + 1]}, row_den}.get_d()};
        }
        for (int col_idx = 0; col_idx < cols; ++col_idx)
        {
            // col_strategy[col_idx] = Real{mpz_get_ui(col_solution_data[col_idx + 1]) / static_cast<typename Real::type>(col_den)};
            col_strategy[col_idx] = Real{mpq_class{mpz_class{col_solution_data[col_idx + 1]}, col_den}.get_d()};
        }

        // const Real row_payoff{mpz_get_ui(col_solution_data[cols + 1]) / static_cast<typename Real::type>(col_den)};
        // const Real col_payoff{mpz_get_ui(row_solution_data[rows + 1]) / static_cast<typename Real::type>(row_den)};
        Real row_payoff{mpq_class{mpz_class{col_solution_data[cols + 1]}, col_den}.get_d()};
        Real col_payoff{mpq_class{mpz_class{row_solution_data[rows + 1]}, row_den}.get_d()};
        row_payoff = row_payoff * range + min;
        col_payoff = col_payoff * range + min;
        delete[] row_solution_data;
        delete[] col_solution_data;

        return {row_payoff, col_payoff};
    }

}; // End namespace LRSNash
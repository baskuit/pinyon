#pragma once

#include "../../extern/lrslib/include/lib.h"

#include <types/types.hh>

namespace LRSNash
{

    template <typename Real, template <typename> typename Vector, template <typename> typename Matrix>
    void solve(Matrix<Real> &payoff_matrix, Vector<Real> &row_strategy, Vector<Real> &col_strategy)
    {
    }

    template <template <typename> typename Vector, template <typename> typename Matrix, int num, int den>
    void solve(
        Matrix<ValueStruct<RealType<mpq_class>, false, num, den>> &payoff_matrix, 
        Vector<RealType<mpq_class>> &row_strategy, 
        Vector<RealType<mpq_class>> &col_strategy)
    {
        const size_t rows = payoff_matrix.rows;
        const size_t cols = payoff_matrix.cols;
        const size_t entries = rows * cols;

        mpq_t *row_payoff_data = new mpq_t[entries];
        mpq_t *col_payoff_data = new mpq_t[entries];
        for (size_t i = 0; i < entries; ++i) {
            mpq_init(row_payoff_data[i]);
            mpq_init(col_payoff_data[i]);

            mpq_set(row_payoff_data[i], payoff_matrix[i].get_row_value().unwrap().get_mpq_t());
            mpq_set(col_payoff_data[i], payoff_matrix[i].get_col_value().unwrap().get_mpq_t());
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

        solve_gmp(rows, cols, row_payoff_data, col_payoff_data, row_solution_data, col_solution_data);

        mpz_class row_den{row_solution_data[0]}, col_den{col_solution_data[0]};
        for (int row_idx = 0; row_idx < rows; ++row_idx)
        {
            row_strategy.emplace_back(mpq_class{mpz_class{row_solution_data[row_idx + 1]}, row_den});
        }
        for (int col_idx = 0; col_idx < cols; ++col_idx)
        {
            col_strategy.emplace_back(mpq_class{mpz_class{col_solution_data[col_idx + 1]}, col_den});
        }
        delete[] row_solution_data;
        delete[] col_solution_data;
    }

}; // End namespace LRSNash
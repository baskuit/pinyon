#pragma once

#include "../../extern/lrslib/include/lib-gmp.h"
#include "../../extern/lrslib/include/lib-long.h"

#include <libsurskit/math.hh>

#include <gmpxx.h>

/*

128 bit works just fine for Taurus use case but other games may require mpz_t

TODO:

Make the interface independent of underlying int type

Make it so that changing the type is not done with branches but instead with compiler flag


*/

namespace LRSNash
{

    void solve_matrix(
        Matrix<PairDouble> &payoff_matrix,
        std::vector<double> &row_strategy,
        std::vector<double> &col_strategy,
        const size_t n_discrete = 100)
    {
        const size_t rows = payoff_matrix.rows;
        const size_t cols = payoff_matrix.cols;
        row_strategy.resize(rows);
        col_strategy.resize(cols);

        const auto min = payoff_matrix.min();
        const auto max = payoff_matrix.max();
        auto delta = (max - min);

        Matrix<PairDouble> normalized{rows, cols};
        int idx = 0;
        for (const auto value : payoff_matrix)
        {
            normalized[idx].row_value = static_cast<double>((value.get_row_value() - min) / delta);
            normalized[idx].col_value = static_cast<double>((value.get_col_value() - min) / delta);
            ++idx;
        }

        std::array<int, 81> row_num, row_den, col_num, col_den;

        idx = 0;
        for (const auto value : normalized)
        {
            row_num[idx] = floor(value.get_row_value() * n_discrete);
            col_num[idx] = floor(value.get_col_value() * n_discrete);
            row_den[idx] = n_discrete;
            col_den[idx] = n_discrete;

            ++idx;
        }

        game g;
        init_game_long(&g, rows, cols, row_num.data(), row_den.data(), col_num.data(), col_den.data());

        auto row_data = alloc_data_long(rows + 2);
        auto col_data = alloc_data_long(cols + 2);

        solve_long(&g, row_data, col_data);

        double x{1 / static_cast<double>(*row_data[0])};
        for (int row_idx = 0; row_idx < rows; ++row_idx)
        {
            row_strategy[row_idx] = *row_data[row_idx + 1] * x;
        }

        double y{1 / static_cast<double>(*col_data[0])};

        for (int col_idx = 0; col_idx < cols; ++col_idx)
        {
            col_strategy[col_idx] = *col_data[col_idx + 1] * y;
        }

        dealloc_data_long(row_data, rows + 2);
        dealloc_data_long(col_data, cols + 2);
    }

    void solve_matrix(
        Matrix<PairRational> &payoff_matrix,
        std::vector<mpq_class> &row_strategy,
        std::vector<mpq_class> &col_strategy)
    {
        const size_t rows = payoff_matrix.rows;
        const size_t cols = payoff_matrix.cols;
        row_strategy.resize(rows);
        col_strategy.resize(cols);

        std::array<mpq_t, 81> row_payoff_data, col_payoff_data;

        for (int i = 0; i < rows * cols; ++i) {
            *row_payoff_data[i] = *payoff_matrix[i].get_row_value().get_mpq_t();
            *col_payoff_data[i] = *payoff_matrix[i].get_col_value().get_mpq_t();
        }

        auto row_data = alloc_data_gmp(rows + 2);
        auto col_data = alloc_data_gmp(cols + 2);

        game g;

        solve_gmp_2(&g, rows, cols, row_payoff_data.data(), col_payoff_data.data(), row_data, col_data);

        mpz_class x{row_data[0]};
        for (int row_idx = 0; row_idx < rows; ++row_idx)
        {
            mpz_class x_{row_data[row_idx + 1]};
            row_strategy[row_idx] = mpq_class{x_, x};
        }

        mpz_class y{col_data[0]};

        for (int col_idx = 0; col_idx < cols; ++col_idx)
        {
            mpz_class y_{col_data[col_idx + 1]};
            col_strategy[col_idx] = mpq_class{y_, y};
        }

        dealloc_data_gmp(row_data, rows + 2);
        dealloc_data_gmp(col_data, cols + 2);
    }

}; // End namespace LRSNash
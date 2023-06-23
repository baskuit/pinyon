#pragma once

#include "../../extern/lrslib/src/lib.h"

#include <libsurskit/math.hh>

namespace LRSNash
{

    void eee(
        Matrix<PairDouble> &payoff_matrix,
        std::vector<double> &row_strategy,
        std::vector<double> &col_strategy,
        const size_t n_discrete = 100)
    {
        const int rows = payoff_matrix.rows;
        const int cols = payoff_matrix.cols;
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
        init_game(&g, rows, cols, row_num.data(), row_den.data(), col_num.data(), col_den.data());

        auto row_data = alloc_data(rows + 2);
        auto col_data = alloc_data(cols + 2);

        solve(&g, row_data, col_data);

        double x{1 / *row_data[0]};
        for (int row_idx = 0; row_idx < rows; ++row_idx)
        {
            row_strategy[row_idx] = *row_data[row_idx + 1] * x;
        }

        double y{1 / *col_data[0]};

        for (int col_idx = 0; col_idx < cols; ++col_idx)
        {
            col_strategy[col_idx] = *col_data[col_idx + 1] * y;
        }

        dealloc_data(row_data, rows + 2);
        dealloc_data(col_data, cols + 2);
    }


}; // End namespace Gambit
#pragma once

#include "../../extern/lrslib/src/lib.h"

#include <libsurskit/math.hh>

#include <algorithm>

namespace LRSNash
{

    template <class Types>
    void solve_matrix(
        typename Types::MatrixValue &payoff_matrix,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy,
        const size_t n_discrete = 20)
    {
        const int rows = payoff_matrix.rows;
        const int cols = payoff_matrix.cols;
        row_strategy.fill(rows);
        col_strategy.fill(cols);

        const auto min = payoff_matrix.min();
        const auto max = payoff_matrix.max();
        const auto delta = max - min;

        const auto normalized = (payoff_matrix + (min * -1)) / delta;

        std::array<int, rows + cols + 2> row_num, row_den{n_discrete}, col_num, col_den{n_discrete};

        int idx = 0;
        for (const auto value : normalized)
        {
            row_num[idx] = floor(value.get_row_value() * n_discrete);
            col_num[idx] = floor(value.get_col_value() * n_discrete);
            ++idx;
        }

        game g;
        init_game(&g, rows, cols, row_num, row_den, col_num, col_den);

        auto row_data = alloc_data(rows + 2);
        auto col_data = alloc_data(cols + 2);

        solve(&g, row_data, col_data);

        typename Types::Real x{row_data[0]};
        x = 1 / x;
        for (int row_idx = 0; row_idx < rows; ++row_idx) {
            row_strategy[row_idx] = row_data[row_idx + 1] * x;
        }

        typename Types::Real y{col_data[0]};
        y = 1 / y;
        for (int col_idx = 0; col_idx < cols; ++col_idx) {
            col_strategy[col_idx] = col_data[col_idx + 1] * y;
        }

        dealloc_data(row_data, rows + 2);
        dealloc_data(col_data, cols + 2);
    }

}; // End namespace Gambit
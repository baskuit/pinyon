#include "libsurskit/random.hh"
#include "libsurskit/gambit.hh"
#include "state/state.hh"

#include <iostream>
#include <assert.h>

/*

Generate matrices of various sizes, solve using LibGambit, and test that expl is basically zero.

Passes with size = 7, device seed = 0, n_matrices  = 10000
However that takes several minutes, so we will reduce the size of the test.
*/

constexpr int min_dim = 1;
constexpr int size = 6;

using Types = TypeList<
    int,
    int,
    double,
    double,
    Array<int, size>,
    Array<double, size>,
    Array<int, size>,
    Linear::Matrix<double, size>,
    Linear::Matrix<int, size>>;

void random_matrices(prng &device, int rows, int cols, int n_matrices, Types::Real epsilon, double &avg_expl, double &max_expl)
{
    Types::VectorReal row_strategy(rows);
    Types::VectorReal col_strategy(cols);

    double total_expl = 0;

    for (int i = 0; i < n_matrices; ++i)
    {

        Types::MatrixReal row_payoff_matrix(rows, cols);
        Types::MatrixReal col_payoff_matrix(rows, cols);

        for (int row_idx = 0; row_idx < rows; ++row_idx)
        {
            for (int col_idx = 0; col_idx < cols; ++col_idx)
            {
                const Types::Real row_payoff = device.uniform();
                row_payoff_matrix.get(row_idx, col_idx) = row_payoff;
                col_payoff_matrix.get(row_idx, col_idx) = 1 - row_payoff;
            }
        }

        LibGambit::solve_bimatrix<Types>(row_payoff_matrix, col_payoff_matrix, row_strategy, col_strategy);

        const Types::Real expl = Linear::exploitability<Types>(row_payoff_matrix, col_payoff_matrix, row_strategy, col_strategy);
        assert(expl < epsilon);
        total_expl += expl;
        if (expl > max_expl) {
            max_expl = expl;
        }
    }

    avg_expl = total_expl / n_matrices;
}

int main()
{

    prng device(0);
    for (int rows = min_dim; rows <= size; ++rows)
    {
        for (int cols = min_dim; cols <= size; ++cols)
        {
            double avg_expl = 0;
            double max_expl = 0;
            random_matrices(device, rows, cols, 1000, 0.001, avg_expl, max_expl);
            std::cout << "n: " << rows << ", m: " << cols << "| avg_expl=" << avg_expl << ", max_expl=" << max_expl << std::endl;
        }
    }

    return 0;
}
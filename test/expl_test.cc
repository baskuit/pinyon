#include "libsurskit/random.hh"
#include "libsurskit/gambit.hh"
#include "state/state.hh"

#include <iostream>
#include <assert.h>


/*

Generate matrices of various sizes, solve using LibGambit, and test that expl is basically zero.

*/

constexpr int size = 7;

using Types = TypeList<
    int, 
    int, 
    double, 
    double, 
    std::array<int, size>, 
    std::array<double, size>, 
    std::array<int, size>,
    Linear::Matrix<double, size>,
    Linear::Matrix<int, size>>;

void random_matrices (prng &device, int rows, int cols, int n_matrices, double epsilon) {

    std::array<double, size> row_strategy;
    std::array<double, size> col_strategy;

    for (int i = 0; i < n_matrices; ++i) {

        Types::MatrixReal row_payoff_matrix(rows, cols);
        Types::MatrixReal col_payoff_matrix(rows, cols);

        for (int row_idx = 0; row_idx < rows; ++row_idx) {
            for (int col_idx = 0; col_idx < cols; ++col_idx) {
                const double row_payoff = device.uniform();
                row_payoff_matrix.get(row_idx, col_idx) = row_payoff; 
                col_payoff_matrix.get(row_idx, col_idx) = 1 - row_payoff;
            }
        }

        LibGambit::solve_bimatrix<Types>(row_payoff_matrix, col_payoff_matrix, row_strategy, col_strategy);

        const double expl = Linear::exploitability<Types>(row_payoff_matrix, col_payoff_matrix, row_strategy, col_strategy, 1);
        assert(expl < epsilon);
    }
    return;
}


int main () {

    prng device(0);

    for (int rows = 1; rows <= size; ++rows) {
        for (int cols = 1; cols <= size; ++cols) {
            random_matrices(device, rows, cols, 10000, 0.001);
        }
    }

    return 0;
}
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

constexpr int size = 5;

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

void random_matrices (prng &device, int rows, int cols, int n_matrices, double epsilon) {

    Array<double, size> row_strategy;
    Array<double, size> col_strategy;

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

        const double expl = Linear::exploitability<Types>(row_payoff_matrix, col_payoff_matrix, row_strategy, col_strategy);
        assert(expl < epsilon);
    }
    return;
}


int main () {

    prng device(0);

    for (int rows = 1; rows <= size; ++rows) {
        for (int cols = 1; cols <= size; ++cols) {
            random_matrices(device, rows, cols, 1000, 0.001);
        }
    }

    return 0;
}
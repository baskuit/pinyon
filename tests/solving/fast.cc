#include <surskit.hh>

/*

Fast solving on Matrix<PairDouble> using 128bit lrsnash.
Normalizes each matrix into [0, 1] and the quantizes.
Exploitability is bounded by 1 / n_discrete

*/

// Matrix<PairDouble> random_matrix(prng &device, const size_t rows, const size_t cols)
// {
//     Matrix<PairDouble> payoff_matrix{rows, cols};
//     for (auto &value : payoff_matrix)
//     {
//         value.row_value = device.uniform();
//         value.col_value = 1 - value.row_value;
//     }
//     return payoff_matrix;
// }

// void test(prng &device, const size_t num_matrices, const size_t n_discrete)
// {
//     for (size_t rows = 2; rows <= 9; ++rows)
//     {
//         for (size_t cols = 2; cols <= 9; ++cols)
//         {
//             for (size_t matrix_idx = 0; matrix_idx < num_matrices; ++matrix_idx)
//             {
//                 auto payoff_matrix = random_matrix(device, rows, cols);
//                 std::vector<double> row_strategy{}, col_strategy{};
//                 LRSNash::solve_matrix(payoff_matrix, row_strategy, col_strategy, n_discrete);

//                 const double expl = math::exploitability(payoff_matrix, row_strategy, col_strategy);
//                 assert(expl * n_discrete > 1);
//             }
//         }
//     }
// }

int main()
{
    // prng device{};
    // const size_t num_matrices = 10000;

    // std::cout << "device seed: " << device.get_seed() << std::endl;
    // std::vector<size_t> n_discrete_vec = {10, 20, 50, 100};
    // for (const size_t n_discrete : n_discrete_vec)
    // {
    //     std::cout << "testing n_discrete = " << n_discrete << std::endl;
    //     test(device, num_matrices, n_discrete);
    // }

    return 0;
}
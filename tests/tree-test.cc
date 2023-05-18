#include "../src/state/random-tree.hh"
#include "../src/model/model.hh"
#include "../src/algorithm/bandits/exp3.hh"
#include "../src/algorithm/bandits/matrix-ucb.hh"
#include "../src/algorithm/solve/full-traversal.hh"
#include "../src/algorithm/solve/alphabeta.hh"

#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

using namespace std::chrono;

std::ofstream file("test.txt", std::ios::app);

void get_expl_from_random_tree_loop(
    uint64_t device_seed,
    size_t depth_bound,
    size_t rows,
    size_t cols,
    size_t transitions,
    double chance_threshold,
    size_t max_log2_iterations)
{
    using State = RandomTree;
    using Model = MonteCarloModel<State>;
    using Exp3 = Exp3<Model, TreeBandit>;
    using MatrixUCB = MatrixUCB<Model, TreeBandit>;

    prng device(device_seed);
    State state(device, depth_bound, rows, cols, transitions, chance_threshold);
    Model model(device);
    Exp3 exp3_session;
    MatrixUCB matrix_ucb_session;
    FullTraversal<Model> session;

    MatrixNode<Exp3> exp3_root;
    MatrixNode<MatrixUCB> matrix_ucb_root;

    MatrixNode<FullTraversal<Model>> root;
    session.run(state, model, &root);
    auto row_payoff_matrix = root.stats.nash_payoff_matrix;
    auto col_payoff_matrix = row_payoff_matrix * -1 + 1;

    size_t iterations = 1 << 8;
    auto start = high_resolution_clock::now();
    exp3_session.run(iterations, device, state, model, exp3_root);
    auto end = high_resolution_clock::now();
    auto exp3_duration = duration_cast<microseconds>(end - start);

    start = high_resolution_clock::now();
    matrix_ucb_session.run(iterations, device, state, model, matrix_ucb_root);
    end = high_resolution_clock::now();
    auto matrix_ucb_duration = duration_cast<microseconds>(end - start);

    State::Types::VectorReal row_strategy, col_strategy;
    while (iterations <= (1 << max_log2_iterations))
    {

        exp3_session.get_empirical_strategies(&exp3_root, row_strategy, col_strategy);
        auto exp3_expl = Linear::exploitability<State::Types>(
            row_payoff_matrix,
            col_payoff_matrix,
            row_strategy,
            col_strategy);

        matrix_ucb_session.get_empirical_strategies(&matrix_ucb_root, row_strategy, col_strategy);
        auto matrix_ucb_expl = Linear::exploitability<State::Types>(
            row_payoff_matrix,
            col_payoff_matrix,
            row_strategy,
            col_strategy);

        file << "exp3,"       << device_seed << ',' << depth_bound << ',' << rows << ',' << transitions << chance_threshold << ',' << iterations << ',' <<       exp3_root.count_matrix_nodes() << ',' <<       exp3_expl << ',' <<       exp3_duration << '\n';
        file << "matrix_ucb," << device_seed << ',' << depth_bound << ',' << rows << ',' << transitions << chance_threshold << ',' << iterations << ',' << matrix_ucb_root.count_matrix_nodes() << ',' << matrix_ucb_expl << ',' << matrix_ucb_duration << '\n';

        start = high_resolution_clock::now();
        exp3_session.run(iterations, device, state, model, exp3_root);
        end = high_resolution_clock::now();
        exp3_duration += duration_cast<microseconds>(end - start);

        start = high_resolution_clock::now();
        matrix_ucb_session.run(iterations, device, state, model, matrix_ucb_root);
        end = high_resolution_clock::now();
        matrix_ucb_duration += duration_cast<microseconds>(end - start);

        iterations <<= 1;
    }
}

void foo(
    const uint64_t initial_seed,
    const size_t max_depth_bound,
    const size_t max_actions,
    const size_t max_transitions,
    const size_t max_log2_iterations,
    const size_t max_games)
{
    prng device(initial_seed);
    for (size_t depth_bound = 4; depth_bound <= max_depth_bound; ++depth_bound)
    {
        for (size_t rows = 3; rows <= max_actions; ++rows)
        {
            size_t cols = rows;
            for (size_t transitions = 1; transitions <= max_transitions; ++transitions)
            {
                for (size_t game = 0; game <= max_games; ++game)
                {
                    const uint64_t tree_seed = device.uniform_64();
                    const double transition_threshold = 1 / (double)(transitions + 1);
                    std::cout << tree_seed << ' ' << depth_bound << ' ' << rows << ' ' << transitions << ' ' << transition_threshold << '\n';
                    get_expl_from_random_tree_loop(tree_seed, depth_bound, rows, cols, transitions, transition_threshold, max_log2_iterations);
                }
            }
        }
    }
}

int main()
{

    foo(0, 6, 4, 2, 17, 10);

    return 0;
}
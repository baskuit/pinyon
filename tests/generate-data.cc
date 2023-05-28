#include <state/random-tree.hh>
#include <model/model.hh>
#include <algorithm/tree-bandit/tree/multithreaded.hh>
#include <algorithm/tree-bandit/bandit/exp3.hh>
#include<algorithm/tree-bandit/bandit/matrix-ucb.hh>
#include <algorithm/solver/full-traversal.hh>
#include <algorithm/solver/alpha-beta.hh>


#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <mutex>

using namespace std::chrono;

std::ofstream file("data.txt", std::ios::app);
std::mutex file_mutex;

struct GameData
{

    size_t matrix_node_count = 0;
    size_t time_spent = 0;
    size_t n = 0;

    GameData() {}

    GameData(size_t matrix_node_count, size_t time_spent) : matrix_node_count(matrix_node_count), time_spent(time_spent), n(1) {}

    GameData &operator+=(const GameData &other)
    {
        matrix_node_count += other.matrix_node_count;
        time_spent += other.time_spent;
        n += other.n;
        return (*this);
    }
};

template <class RowAlgorithm, class ColAlgorithm>
typename RowAlgorithm::Types::Real vs(
    typename RowAlgorithm::Types::PRNG &device,
    size_t iterations,
    typename RowAlgorithm::Types::State &state,
    typename RowAlgorithm::Types::Model &row_model,
    typename ColAlgorithm::Types::Model &col_model,
    RowAlgorithm &row_session,
    ColAlgorithm &col_session,
    GameData &row_data,
    GameData &col_data)
{
    using State = typename RowAlgorithm::Types::State;
    auto state_copy = state;
    state_copy.get_actions();

    while (!state_copy.is_terminal)
    {

        ActionIndex row_idx, col_idx;

        MatrixNode<RowAlgorithm> row_root;
        MatrixNode<ColAlgorithm> col_root;

        auto start = high_resolution_clock::now();
        row_session.run(iterations, device, state_copy, row_model, row_root);
        auto end = high_resolution_clock::now();
        auto row_time_spent = duration_cast<microseconds>(end - start);

        start = high_resolution_clock::now();
        col_session.run(iterations, device, state_copy, col_model, col_root);
        end = high_resolution_clock::now();
        auto col_time_spent = duration_cast<microseconds>(end - start);

        typename RowAlgorithm::Types::VectorReal row_strategy, col_strategy;
        row_session.get_empirical_strategies(&row_root, row_strategy, col_strategy);
        row_idx = device.sample_pdf(row_strategy);
        col_session.get_empirical_strategies(&col_root, row_strategy, col_strategy);
        col_idx = device.sample_pdf(col_strategy);
        state_copy.apply_actions(state_copy.row_actions[row_idx], state_copy.col_actions[col_idx]);
        state_copy.get_actions();

        GameData row_game_data(row_root.count_matrix_nodes(), row_time_spent.count());
        GameData col_game_data(col_root.count_matrix_nodes(), col_time_spent.count());
        row_data += row_game_data;
        col_data += col_game_data;
    }

    return state_copy.row_payoff;
}

void file_write(
    const std::string name,
    const uint64_t seed,
    const size_t depth_bound,
    const size_t actions,
    const size_t transitions,
    const double chance_threshold,
    const size_t iterations,
    double average_score,
    const size_t matrix_node_count,
    const size_t time_spent,
    const size_t sample_size,
    const size_t total_games)
{

    file_mutex.lock();

    file
        << name << ','
        << seed << ','
        << depth_bound << ','
        << actions << ','
        << transitions << ','
        << chance_threshold << ','
        << iterations << ','
        << average_score << ','
        << matrix_node_count << ','
        << time_spent << ','
        << sample_size << ','
        << total_games << std::endl;

    file_mutex.unlock();
}

void foo(
    const uint64_t initial_seed,
    std::vector<size_t> &depth_bound,
    std::vector<size_t> &actions,
    std::vector<size_t> &transitions,
    std::vector<size_t> &log2_iterations,
    const size_t max_games)
{

    /*

    Generates new games

    */

    using State = RandomTree;
    using Model = MonteCarloModel<State>;
    using Exp3 = Exp3<Model, TreeBandit>;
    using MatrixUCB = MatrixUCB<Model, TreeBandit>;

    prng device(initial_seed);
    Model model(device);
    Exp3 exp3_session;
    MatrixUCB matrix_ucb_session;

    // exp3_session.threads = 3;
    // matrix_ucb_session.threads = 3;

    for (size_t cycle = 0; cycle < 100; ++cycle)
    {
        for (const size_t db : depth_bound)
        {
            for (const size_t rows : actions)
            {
                size_t cols = rows;
                for (const size_t t : transitions)
                {
                    double chance_threshold = 1 / (double)(t + 1);

                    for (const size_t it : log2_iterations)
                    {
                        const size_t iterations = 1 << it;
                        double exp3_cum_score = 0;
                        GameData exp3_data, matrix_ucb_data;
                        uint64_t state_seed = device.uniform_64();
                        prng state_device(state_seed);
                        State state(state_device, db, rows, cols, t, chance_threshold);

                        for (size_t game_on_fixed_tree = 0; game_on_fixed_tree <= max_games; ++game_on_fixed_tree)
                        {
                            exp3_cum_score += vs(device, iterations, state, model, model, exp3_session, matrix_ucb_session, exp3_data, matrix_ucb_data);
                            exp3_cum_score += 1 - vs(device, iterations, state, model, model, matrix_ucb_session, exp3_session, matrix_ucb_data, exp3_data);
                        }

                        file_write(
                            "exp3",
                            state_seed, db, rows, t, chance_threshold, iterations, exp3_cum_score / max_games / 2,
                            exp3_data.matrix_node_count / exp3_data.n,
                            exp3_data.time_spent / exp3_data.n,
                            exp3_data.n,
                            2 * max_games);

                        file_write(
                            "matrix_ucb",
                            state_seed, db, rows, t, chance_threshold, iterations, (max_games * 2 - exp3_cum_score) / max_games / 2,
                            matrix_ucb_data.matrix_node_count / matrix_ucb_data.n,
                            matrix_ucb_data.time_spent / matrix_ucb_data.n,
                            matrix_ucb_data.n,
                            2 * max_games);

                    }
                }
            }
        }
    }
}

std::vector<size_t> range(const size_t a, const size_t b, const size_t c = 1)
{
    std::vector<size_t> x{};
    for (size_t i = a; i < b; i += c)
    {
        x.push_back(i);
    }
    return x;
}

std::ofstream bmark("b.txt", std::ios::app);


void bar()
{
    prng device;
    uint64_t initial_seed = device.uniform_64();
    file_mutex.lock();
    std::cout << initial_seed << std::endl;
    file_mutex.unlock();
    auto depth_bound = range(5, 25, 3);
    auto actions = range(5, 6);
    auto transitions = range(1, 3);
    auto log2_iterations = range(8, 12);
    size_t max_games = 5;

    foo(initial_seed,
        depth_bound,
        actions,
        transitions,
        log2_iterations,
        max_games);

}

#include <vector>

int main () {

    size_t threads = 4;
    std::vector<std::thread> pool;
    for (size_t thread = 0; thread < threads; ++thread) {
        pool.emplace_back(std::thread(&bar));
    }

    for (size_t thread = 0; thread < threads; ++thread) {
        pool[thread].join();
    }

    return 0;
}
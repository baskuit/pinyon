#include "seed-state.hh"
#include "model/model.hh"
#include "algorithm/exp3p.hh"
#include "algorithm/exp3.hh"
#include "algorithm/matrix-ucb.hh"

#include "grow.hh"

#include <iostream>

template <class Algorithm>
void print_matrix (MatrixNode<Algorithm> *matrix_node) {
    for (int row_idx = 0; row_idx < matrix_node->actions.rows; ++row_idx) {
        for (int col_idx = 0; col_idx < matrix_node->actions.cols; ++col_idx) {
            auto chance_node = matrix_node->access(row_idx, col_idx);
            auto next = chance_node->child;
            double value = 0;
            while (next != nullptr) {
                value += next->inference.row_value;
                next = next->next;
            }
            std::cout << value << ' ';
        }
        std::cout << std::endl;
    }
}

int main()
{

    const int MaxActions = 3;
    const int MaxTransitions = 2;

    using SeedState = SeedState<MaxActions, MaxTransitions>;
    using Model = MonteCarloModel<SeedState>;
    using MatrixUCB = MatrixUCB<Model, TreeBandit>;
    using Exp3p = Exp3p<Model, TreeBandit>;
    using Algorithm = MatrixUCB;

    prng init_device;
    int seed = 76874;//init_device.random_int(100000);
    prng device(seed);

    SeedState state(device, 2, 3, 3, nullptr, nullptr, nullptr);
    Model model(device);

    // Algorithm session;
    // MatrixNode<Algorithm> root;
    // session.run(1000, device, state, model, root);

    // SeedState::Types::MatrixReal row_value_matrix(3, 3), col_value_matrix(3, 3);
    // session.get_ev_matrix(&root, row_value_matrix, col_value_matrix);
    // std::cout << "expected value matrix:\n";
    // row_value_matrix.print();

    // SeedState::Types::VectorReal row_strategy(3), col_strategy(3);
    // session.get_strategies(&root, row_strategy, col_strategy);
    // std::cout << "strategies:\n";
    // math::print(row_strategy, 3);
    // math::print(col_strategy, 3);

    /*
    */

    Grow<Model> grow;
    MatrixNode<Grow<Model>> grow_root;
    grow.grow(state, model, &grow_root);

    auto row_solution = grow_root.inference.row_policy;
    auto col_solution = grow_root.inference.col_policy;

    std::cout << "node count: " << grow_root.stats.matrix_node_count << '\n';
    std::cout << "row payoff matrix:\n";
    SeedState::Types::MatrixReal row_payoff_matrix = grow_root.stats.nash_payoff_matrix;
    SeedState::Types::MatrixReal col_payoff_matrix = row_payoff_matrix * -1 + 1;
    row_payoff_matrix.print();
    std::cout << "solutions:\n";
    math::print(row_solution, 3);
    math::print(col_solution, 3);

    

    // double expl = Linear::exploitability<SeedState::Types::TypeList>(
    //     row_payoff_matrix, 
    //     col_payoff_matrix, 
    //     row_strategy, 
    //     col_strategy);
    std::cout << "session expl: " << expl << '\n';

    for (auto c :state.transition_strategies) {
        std::cout << c << ' ';
    }
    std::cout << std::endl;
    std::cout << "seed: " << seed << std::endl;

    

    return 0;
}
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
    const int MaxTransitions = 1;

    using SeedState = SeedState<MaxActions, MaxTransitions>;
    using Model = MonteCarloModel<SeedState>;
    using MatrixUCB = MatrixUCB<Model, TreeBandit>;

    prng device;

    SeedState state(device, 1, 3, 3, nullptr, nullptr, nullptr);
    Model model(device);
    MatrixUCB session;
    MatrixNode<MatrixUCB> root;

    session.run(10, device, state, model, root);

    MatrixUCB::Types::VectorReal row_strategy(3), col_strategy(3);
    MatrixUCB::Types::MatrixReal row_value_matrix(3, 3), col_value_matrix(3, 3);

    session.get_ev_matrix(&root, row_value_matrix, col_value_matrix);

    std::cout << "row ev matrix:\n";
    row_value_matrix.print();
    print_matrix(&root);
    session.get_strategies(&root, row_strategy, col_strategy);
    std::cout << "strategies:\n";
    math::print(row_strategy, 3);
    math::print(col_strategy, 3);



    // double expl = Linear::exploitability<SeedState::Types::TypeList>(
    //     row_payoff_matrix, 
    //     col_payoff_matrix, 
    //     row_strategy, 
    //     col_strategy);
    // std::cout << "session expl: " << expl << '\n';
    /*
    Bug cus no call get_actions to get transition_strategies
    Where should we put the get_actions call to guarantee no search is done on state that doesnt have actions?
    If put at run, then
    */

    Grow<Model> grow;
    MatrixNode<Grow<Model>> grow_root;
    grow.grow(state, model, &grow_root);

    auto row_solution = grow_root.inference.row_policy;
    auto col_solution = grow_root.inference.col_policy;

    std::cout << "node count: " << grow_root.stats.matrix_node_count << '\n';
    std::cout << "row payoff matrix:\n";
    MatrixUCB::Types::MatrixReal row_payoff_matrix = grow_root.stats.nash_payoff_matrix;
    MatrixUCB::Types::MatrixReal col_payoff_matrix = row_payoff_matrix * -1 + 1;
    row_payoff_matrix.print();
    std::cout << "solutions:\n";
    math::print(row_solution, 3);
    math::print(col_solution, 3);

    

    return 0;
}
#include "seed-state.hh"
#include "model/model.hh"
#include "algorithm/exp3p.hh"
#include "algorithm/exp3.hh"
#include "algorithm/matrix-ucb.hh"

#include "grow.hh"
#include "tree-state.hh"
#include "alphabeta.hh"

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

    const int MaxActions = 4;
    const int MaxTransitions = 1;
    const double chance_threshold = 1 / (double) 3; 

    using SeedState = SeedState<MaxActions, MaxTransitions>;
    using Model = MonteCarloModel<SeedState>;
    using MatrixUCB = MatrixUCB<Model, TreeBandit>;
    using Exp3p = Exp3p<Model, TreeBandit>;
    using Algorithm = Exp3p;

    SeedState::Types::VectorReal row_strategy(MaxActions), col_strategy(MaxActions);
    SeedState::Types::MatrixReal row_value_matrix(MaxActions, MaxActions), col_value_matrix(MaxActions, MaxActions);

    prng init_device;
    int seed = 100;//init_device.uniform_64();
    std::cout << "seed: " << seed << std::endl;
    prng device(seed);
    int games = 10;


    for (int actions = 2; actions <= MaxActions; ++actions) {
        for (int depth_bound = 1; depth_bound < 2; ++depth_bound) {


            double total_exp3p_expl = 0;
            double total_matrix_ucb_expl = 0;

            for (int game = 0; game < games; ++game) {

                SeedState state(device, depth_bound, actions, actions, chance_threshold);
                Model model(device);
                TreeState<Model> tree_state(state, model);

                AlphaBeta<Model> alpha_beta_session(0, 1);
                alpha_beta_session.run(state, model);
                std::cout << "TreeState value: " << tree_state.current_node->stats.row_payoff << '\n';
                return 0;
            }

        }
    }

    return 0;
}
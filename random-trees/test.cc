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

template <class State>
void alpha_beta_test (State &state) {
    using Model = MonteCarloModel<State>;
    TreeState<Model> tree_state(state)
}

template <size_t MaxActions, size_t MaxTransitions>
void alpha_beta_test (
    const int min_actions, const int max_actions,
    const int min_depth, const int max_depth,
    const int games, const uint64_t seed = 0) 
{
    using RandomTree = RandomTree<MaxActions, MaxTransitions>;
    using Model = MonteCarloModel<RandomTree>;
    using MatrixUCB = MatrixUCB<Model, TreeBandit>;
    using Exp3p = Exp3p<Model, TreeBandit>;
    using Algorithm = Exp3p;

    prng device(seed);
    Rational chance_threshold = Rational(0);

    for (int actions = min_actions; actions <= max_actions; ++actions) {
        for (int depth_bound = min_depth; depth_bound <= max_depth; ++depth_bound) {

            double total_proportion = 0;

            for (int game = 0; game < games; ++game) {
                const uint64_t new_seed = device.uniform_64();
                prng new_device(new_seed);
                RandomTree state(new_device, depth_bound, actions, actions, chance_threshold);
                Model model(new_device);
                TreeState<Model> tree_state(state, model);
                AlphaBeta<Model> alpha_beta_session(0, 1);
                MatrixNode<AlphaBeta<Model>> alpha_beta_root;
                alpha_beta_session.run(state, model, &alpha_beta_root, tree_state.current_node);
                auto a = alpha_beta_root.stats.value;
                auto b = tree_state.current_node->stats.row_payoff;
                const int alpha_beta_count = alpha_beta_root.count_matrix_nodes();
                total_proportion += alpha_beta_count / (double) tree_state.current_node->stats.matrix_node_count;

                assert(alpha_beta_session.equals(a, b));

            }

            std::cout << "average tree size ratio for actions, depth_bound: " << actions << ' ' << depth_bound << " = " << total_proportion / games << std::endl;

        }
    }

}

int main()
{

    alpha_beta_test<7, 1>(5, 7, 4, 5, 10);

    return 0;
}
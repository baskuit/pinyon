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

template <size_t MaxActions, size_t MaxTransitions>
void alpha_beta_test (
    const int min_depth, const int max_depth,
    const int games, const uint64_t seed = 0) 
{
    using SeedState = SeedState<MaxActions, MaxTransitions>;
    using Model = MonteCarloModel<SeedState>;
    using MatrixUCB = MatrixUCB<Model, TreeBandit>;
    using Exp3p = Exp3p<Model, TreeBandit>;
    using Algorithm = Exp3p;

    prng device(seed);
    Rational chance_threshold = Rational(0);

    for (int actions = 2; actions <= MaxActions; ++actions) {
        for (int depth_bound = min_depth; depth_bound <= max_depth; ++depth_bound) {

            double total_exp3p_expl = 0;
            double total_matrix_ucb_expl = 0;

            for (int game = 0; game < games; ++game) {
                const uint64_t new_seed = device.uniform_64();
                // const uint64_t seed = 10641954317717599816;
                prng new_device(new_seed);
                SeedState state(new_device, depth_bound, actions, actions, chance_threshold);
                Model model(new_device);
                TreeState<Model> tree_state(state, model);
                AlphaBeta<Model> alpha_beta_session(0, 1);
                MatrixNode<AlphaBeta<Model>> alpha_beta_root;
                alpha_beta_session.run(state, model, &alpha_beta_root, tree_state.current_node);
                auto a = alpha_beta_root.stats.value;
                auto b = tree_state.current_node->stats.row_payoff;
                std::cout << a << ' ' << b << std::endl;
                // assert(alpha_beta_session.equals(a, b));

            }

        }
    }

}

int main()
{

    alpha_beta_test<5, 1>(1, 5, 100);

    return 0;
}
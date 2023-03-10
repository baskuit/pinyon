#include "libsurskit/random.hh"
#include "../random_trees/tree_state.hh"
#include "search/exp3p.hh"
#include "search/matrix_ucb.hh"
#include "tree/node.hh"


#include <iostream>
#include <concepts>

template <class Algorithm1, class Algorithm2>
class DualAlgorithm {
    public:
    // static_assert(std::is_same<typename Algorithm1::Types::State, typename Algorithm2::Types::State>);
    using State = typename Algorithm1::Types::State;
    using Model1 = typename Algorithm1::Types::Model;
    using Model2 = typename Algorithm2::Types::Model;

    prng& device1;
    prng& device2;

    Model1 model1 = Model1(device1);
    Model2 model2 = Model2(device2);

    Algorithm1 session1 = Algorithm1(device1);
    Algorithm2 session2 = Algorithm2(device2);

    DualAlgorithm (prng &device1, prng &device2) : device1(device1), device2(device2) {}

    double selfplay (State &state, int playouts, double &score) {
        
        // state.get_actions();
        while (!state.is_terminal) {
            MatrixNode<Algorithm1> root1;
            MatrixNode<Algorithm2> root2;

            session1.run(playouts, state, model1, root1);
            session2.run(playouts, state, model2, root2);

            typename State::Types::VectorReal row_strategy;
            typename State::Types::VectorReal col_strategy;

            session1.get_strategies(&root1, row_strategy, col_strategy);
            int idx1 = device1.sample_pdf(row_strategy, state.actions.rows);
            session2.get_strategies(&root2, row_strategy, col_strategy);
            int idx2 = device2.sample_pdf(row_strategy, state.actions.cols);
            state.apply_actions(
                state.actions.row_actions[idx1],
                state.actions.col_actions[idx2]
            );
            state.get_actions();
        }
        return state.row_payoff;
    }

    double selfplay_loop (State &state, int games, int playouts) {
        double score = 0;
        for (int game = 0; game < games; ++game) {
            auto state_ = state;
            score += selfplay(state_, playouts, score);
        }
        return score;
    }

};

int main () {
    using TreeState = TreeState<3>;
    using Model = MonteCarloModel<TreeState>;
    using Algorithm1 = Exp3p<Model, TreeBandit>;
    using Algorithm2 = Algorithm1;

    prng device;

    TreeState tree(device, 1, 3, 3);

    std::cout << "Tree generated" << std::endl;

    DualAlgorithm<Algorithm1, Algorithm2> eval(device, device);

    // eval.session1.c_uct = 2.01;

    double result = eval.selfplay_loop(tree, 100, 2);
    std::cout << result << std::endl;;

    return 0;
}
#include "model/model.hh" //TODO this must be first otherwise include error (algo.hh wants model.hh...) fix with cmake!!!
#include "../random_trees/seed_state.hh"
#include "../random_trees/grow.hh"
#include "../random_trees/tree_state.hh"

#include "libsurskit/math.hh"
#include "algorithm/matrix_ucb.hh"
#include "algorithm/exp3p.hh"

#include <iostream>

const int size = 3;

/*
Currently Broken!!!!
MatrixUCB seems to be solving it just fine but the TreeStates strategies are wrong...
*/

int main()
{

    using TreeState = TreeState<size>;
    using MonteCarlo = MonteCarloModel<TreeState>;

    prng device;
    TreeState state(device, 1, size, size, nullptr, nullptr);
    state.get_actions();
    // Initialization now runs the Grow algorithm automatically

    std::cout << "tree count: " << state.current->stats.count << std::endl;
    std::cout << "root expected value matrix" << std::endl;
    state.current->stats.expected_value.print();
    std::cout << "strategies" << std::endl;
    math::print(state.row_strategy, size);
    math::print(state.col_strategy, size);

    MonteCarlo model(device);
    using Exp3p = Exp3p<MonteCarlo, TreeBandit>;
    MatrixNode<Exp3p> root;
    Exp3p session;

    session.run(100000, device, state, model, root);
    TreeState::Types::VectorReal row_strategy(size);
    TreeState::Types::VectorReal col_strategy(size);
    TreeState::Types::MatrixReal matrix(size, size);

    session.get_strategies(&root, row_strategy, col_strategy);
    std::cout << "get_strategies: " << std::endl;
    math::print(row_strategy, state.actions.rows);
    math::print(col_strategy, state.actions.cols);
    // session.get_ev_matrix(&root, matrix);
    // matrix.print();

    auto col_expected_value = state.current->stats.expected_value * -1 + 1;
    double expl = Linear::exploitability<TreeState::Types>
        (state.current->stats.expected_value, col_expected_value, row_strategy, col_strategy);
    std::cout << "expl: " << expl << std::endl;
    return 0;
}
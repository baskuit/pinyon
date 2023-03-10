#include "model/model.hh" //TODO this must be first otherwise include error (algo.hh wants model.hh...) fix with cmake!!!
#include "../random_trees/seed_state.hh"
#include "../random_trees/grow.hh"
#include "../random_trees/tree_state.hh"

#include "search/matrix_ucb.hh"
#include "search/exp3p.hh"

#include <iostream>

const int __size__ = 3;

int main()
{

    using TreeState = TreeState<__size__>;
    using MonteCarlo = MonteCarloModel<TreeState>;

    prng device;
    TreeState state(device, 1, __size__, __size__);
    state.get_actions();
    // Initialization now runs the Grow algorithm automatically

    std::cout << "tree count: " << state.current->stats.count << std::endl;
    std::cout << "root expected value matrix" << std::endl;
    state.current->stats.expected_value.print();
    std::cout << "strategies" << std::endl;
    math::print(state.row_strategy, __size__);
    math::print(state.col_strategy, __size__);

    MonteCarlo model(device);
    using Exp3p = Exp3p<MonteCarlo, TreeBandit>;
    MatrixNode<Exp3p> root;
    Exp3p session(device);

    session.run(800, state, model, root);
    typename TreeState::Types::VectorReal row_strategy = {0};
    typename TreeState::Types::VectorReal col_strategy = {0};

    session.get_strategies(&root, row_strategy, col_strategy);
    std::cout << "get_strategies: " << std::endl;
    math::print(row_strategy, state.actions.rows);
    math::print(col_strategy, state.actions.cols);

    double x = Linear::exploitability<
        double,
        typename TreeState::Types::MatrixReal,
        typename TreeState::Types::VectorReal>
        (state.current->stats.expected_value, row_strategy, col_strategy);
    std::cout << "expl: " << x << std::endl;
    return 0;
}
#include "model/model.hh" //TODO this must be first otherwise include error (algo.hh wants model.hh...) fix with cmake!!!
#include "../random_trees/seed-state.hh"
#include "../random_trees/grow.hh"
#include "../random_trees/tree-state.hh"

#include "libsurskit/math.hh"
#include "algorithm/matrix-ucb.hh"
#include "algorithm/exp3p.hh"

#include <iostream>

constexpr int size = 3;

int af(prng &device, int actions) {
    const int raw = actions - device.random_int(2) + device.random_int(2);
    return std::max(std::min(raw, size), 1);
}

int dbf(prng &device, int depth_bound) {
    const int raw = depth_bound - device.random_int(2) - 1;
    return std::max(std::min(raw, depth_bound), 0);
}

int main()
{

    using TreeState = TreeState<size>;
    using MonteCarlo = MonteCarloModel<TreeState>;

    prng device;
    TreeState state(device, 4, size, size, &dbf, &af);
    state.get_actions();
    // Initialization now runs the Grow algorithm automatically

    std::cout << "tree count: " << state.current->stats.count << std::endl;
    std::cout << "root expected value matrix" << std::endl;
    state.current->stats.expected_value.print();
    std::cout << "root nash equilibrium" << std::endl;
    math::print(state.row_strategy, size);
    math::print(state.col_strategy, size);

    MonteCarlo model(device);
    using Exp3p = Exp3p<MonteCarlo, TreeBandit>;
    MatrixNode<Exp3p> root;
    Exp3p session;

    const int playouts = 10000;
    session.run(playouts, device, state, model, root);

    std::cout << "Algorithm: " << session << "ran for " << playouts << " playouts" << std::endl;

    TreeState::Types::VectorReal row_strategy(size);
    TreeState::Types::VectorReal col_strategy(size);
    session.get_strategies(&root, row_strategy, col_strategy);
    std::cout << "get_strategies: " << std::endl;
    math::print(row_strategy, state.actions.rows);
    math::print(col_strategy, state.actions.cols);

    // TreeState::Types::MatrixReal matrix(size, size);
    // session.get_ev_matrix(&root, matrix);
    // matrix.print();

    auto col_expected_value = state.current->stats.expected_value * -1 + 1;
    double expl = Linear::exploitability<TreeState::Types>
        (state.current->stats.expected_value, col_expected_value, row_strategy, col_strategy);
    std::cout << "expl (at root only): " << expl << std::endl;
    return 0;
}
#include "seed-state.hh"
#include "model/model.hh"
#include "algorithm/exp3p.hh"
#include "grow.hh"

#include <iostream>

int main () {

    const int MaxActions = 3;
    const int MaxTransitions = 2;

    using SeedState = SeedState<MaxActions, MaxTransitions>;
    using Model = MonteCarloModel<SeedState>;
    using Algorithm = Exp3p<Model, TreeBandit>;

    prng device(1);

    SeedState state(device, 1, 3, 3, nullptr, nullptr, nullptr);
    Model model(device);
    Algorithm session;
    MatrixNode<Algorithm> root;

    session.run(1000000, device, state, model, root);

    Grow<Model> session2;
    MatrixNode<Grow<Model>> root2;
    session2.grow(state, model, &root2);
    root2.stats.nash_payoff_matrix.print();

    for (int j = 0; j < 3; ++j) {
    for (int i = 0; i < 3; ++i) {
        auto state_copy = state;
        state_copy.get_actions();
        state_copy.apply_actions(j, i);
        std::cout << state_copy.row_payoff << '\n';
    }
    } // Clearly shows what's wrong!

    return 0;
}
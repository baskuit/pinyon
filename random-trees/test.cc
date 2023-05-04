#include "seed-state.hh"
#include "model/model.hh"
#include "algorithm/exp3p.hh"

#include <iostream>

int main () {

    const int MaxActions = 3;
    const int MaxTransitions = 2;

    using SeedState = SeedState<MaxActions, MaxTransitions>;
    using Model = MonteCarloModel<SeedState>;
    using Algorithm = Exp3p<Model, TreeBandit>;

    prng device(0);

    SeedState state(device, 1, 3, 3, nullptr, nullptr);
    Model model(device);
    Algorithm session;
    MatrixNode<Algorithm> root;

    session.run(1000, device, state, model, root);

    return 0;
}
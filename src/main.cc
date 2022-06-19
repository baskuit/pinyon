#include <assert.h>
#include <iostream>

#include "state/state.hh"
#include "model/monte_carlo.hh"
#include "tree/node.hh"
#include "search/exp3p.hh"

int main () {

    using State = ToyState<9>;
    using MonteCarlo = MonteCarlo<State>;
    using Exp3p = Exp3p<MonteCarlo>;

    prng device;

    MonteCarlo model(device);
    

    Exp3p session;

    // MatrixNode<Exp3p> root(nullptr, nullptr, TransitionData(0, Rational()));
    MatrixNode<Exp3p> root;

    double total = 0;
    int playouts = 1000000;

    for (int p = 0; p < playouts; ++p) {
        State toy(device, 'u', 2, 0);
        State::pair_actions_t pair;
        model.inference(toy, pair);
        total += model.inference_.value_estimate0;
    }

    std::cout << (total / playouts) << std::endl;

    return 0;
}

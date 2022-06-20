#include <assert.h>
#include <iostream>

#include "state/toy_state.hh"
#include "model/monte_carlo.hh"
#include "tree/node.hh"
#include "search/exp3p.hh"

int main () {

    using State = ToyState<9>;
    using MonteCarlo = MonteCarlo<State>;
    using Exp3p = Exp3p<MonteCarlo>;

    prng device;

    MonteCarlo model(device);
    
    Exp3p session(device);

    int playouts = 1000000;
    MatrixNode<Exp3p> root;

    root.stats.t = playouts;

    double total = 0;

    State toy(device, 'u', 4, 0);

    for (int p = 0; p < playouts; ++p) {
        State toy_ = toy;
        session.search(toy_, model, &root);
    }

    std::cout << "s0: ";
    for (int i = 0; i < 2; ++i) {
        std::cout << root.stats.visits0[i] << ' ';
    }
    std::cout << std::endl;

    std::cout << "s1: ";
    for (int i = 0; i < 2; ++i) {
        std::cout << root.stats.visits1[i] << ' ';
    }
    std::cout << std::endl;

    return 0;
}

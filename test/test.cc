#include <assert.h>
#include <iostream>

#include "state/toy_state.hh"
#include "model/monte_carlo.hh"
#include "tree/node.hh"
#include "search/exp3p_thread.hh"

    using ToyState1 = MoldState<9>;
    using MonteCarlo1 = MonteCarlo<ToyState1>;
    using Exp3pT = Exp3p<MonteCarlo1>;

int main () {
    prng device;
    ToyState1 state(device, 6);
    MatrixNode<Exp3pT> root;
    Exp3pT session;

    int threads = 1;
    int playouts = 1000;

    session.search(threads, playouts, state, &root);

    std::cout << std::endl;

    return 0;
}
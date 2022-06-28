#include <assert.h>
#include <iostream>

#include "state/toy_state.hh"
#include "model/monte_carlo.hh"
#include "tree/node.hh"
#include "search/exp3p_thread.hh"



int main () {


    using ToyState = ToyState<9>;
    using MonteCarlo = MonteCarlo<ToyState>;
    using Exp3p = Exp3p<MonteCarlo>;

    prng device;
    ToyState state(device, 'u', 3, 0);
    state.transition(0,0);
    MatrixNode<Exp3p> root;
    Exp3p session;

    int threads = 1;
    int playouts = 10000000;

    session.search(threads, playouts, state, &root);

    return 0;
}
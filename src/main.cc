
#include <assert.h>
#include <iostream>

#include "state/toy_state.hh"
#include "model/monte_carlo.hh"
#include "tree/node.hh"
#include "search/exp3p_thread.hh"



int main () {

    using ToyState = ToyState<9>;
    using MoldState = MoldState<9>;
    using MonteCarlo = MonteCarlo<MoldState>;
    using Exp3p = Exp3p<MonteCarlo>;

    prng device;
    ToyState toy(device, 'u', 3, 0);
    toy.transition(0,0);
    MoldState mold(device, 7);
    mold.transition(0, 0);
    MatrixNode<Exp3p> root;
    Exp3p session;

    int threads = 4;
    int playouts = 1000000;

    session.search(threads, playouts, mold, &root);
    std::cout << "exp3p threads" << std::endl; 

    return 0;
}
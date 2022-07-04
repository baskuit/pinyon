
#include <assert.h>
#include <iostream>

#include "state/toy_state.hh"
#include "model/monte_carlo.hh"
#include "tree/node.hh"
#include "search/exp3p_thread.hh"



int main () {

    using ToyState = ToyState<9>;
    using MoldState = MoldState<9>;
    using MonteCarlo = MonteCarlo<ToyState>;
    using Exp3p = Exp3p<MonteCarlo>;

    prng device;
    ToyState toy(device, 'u', 4, 0);
    toy.transition(0,0);
    // MoldState mold(device, 7);
    // mold.transition(0, 0);
    MatrixNode<Exp3p> root;
    Exp3p session(device);

    // int threads = 4;
    int playouts = 1000000;

    session.search(1 ,playouts, toy, &root);
    Linear::Matrix2D<double, 9> M(2, 2);
    root.matrix(M);
    M.print();

    return 0;
}
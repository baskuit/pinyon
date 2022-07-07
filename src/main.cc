
#include <assert.h>
#include <iostream>

#include "state/toy_state.hh"
#include "model/monte_carlo.hh"
#include "tree/node.hh"
#include "search/exp3p.hh"
#include "search/matrix_ucb_thread_pool.hh"



int main () {

    using ToyState = ToyState<2>;
    using MoldState = MoldState<2>;
    using MonteCarlo = MonteCarlo<ToyState>;
    using Exp3p = Exp3p<MonteCarlo>;
    using MatrixUCB = MatrixUCB<MonteCarlo, 128>;

    prng device;
    prng device_;
    ToyState toy(device, 'w', 2, 2);
    // toy.transition(0,0);
    // MoldState mold(device, 7);
    // mold.transition(0, 0);
    MatrixNode<MatrixUCB> root;
    MatrixUCB session(device);

    MatrixNode<Exp3p> root_;
    Exp3p session_(device_);

    // int threads = 4;
    int playouts = 10000;

    session.search(1, playouts, toy, &root);
    session_.search(playouts, toy, &root_);

    std::cout << root.count() << std::endl;

    return 0;
}
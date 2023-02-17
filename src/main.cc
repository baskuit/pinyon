
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


    // Init prng devices to use in algorithm and in this case state
    prng device;
    prng device_;
    ToyState toy_state(device, 'w', 2, 2);

    
    MatrixNode<MatrixUCB> matrixucb_root;
    MatrixUCB matrixucb_session(device);

    MatrixNode<Exp3p> exp3p_root;
    Exp3p exp3p_session(device_);

    // int threads = 4;
    int playouts = 10000;

    matrixucb_session.search(1, playouts, toy_state, &matrixucb_root);
    exp3p_session.search(playouts, toy_state, &exp3p_root);

    // std::cout << std::format("{}", matrixucb_root.count()) << std::endl;
    return 0;
}
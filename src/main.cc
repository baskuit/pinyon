#include <assert.h>
#include <iostream>

#include "state/toy_state.hh"
#include "model/monte_carlo.hh"
#include "tree/node.hh"
#include "search/exp3p.hh"

#include <thread>
#include <vector>

    using ToyState1 = MoldState<9>;
    using MonteCarlo1 = MonteCarlo<ToyState1>;
    using Exp3pT = Exp3p<MonteCarlo1>;


void search (MatrixNode<Exp3pT>* root, int playouts, int idx) {
    
    root->stats.t = playouts;

    prng device;

    MonteCarlo1 model(device);
    
    Exp3pT session(device);


    // Change State Parameters

    // ToyState1 toy(device, 'u', 4, 2);
    //     toy.transition(0, 0);
    ToyState1 state(device, 6);


    for (int playout = 0; playout < playouts; ++playout) {
        auto state_ = state;
        session.search(state_, model, root);
    }


};

int main () {

        // MAIN IS CURRENTLY NOTHING. SHOULD EVENTUALLY BE  A ROCK
//     MatrixNode<Exp3pT> root;
//     int playouts = 1000000;
//     std::cout << "playouts: " << playouts << std::endl;

// auto t1 = high_resolution_clock::now();



//     search(&root, playouts, 0);

// auto t2 = high_resolution_clock::now();
// double ms_int = duration_cast<milliseconds>(t2 - t1).count();
// std::cout << "time (ms): " << ms_int << std::endl;

//     //std::cout << root.stats.visits0[0] << " " << root.stats.visits0[1] << std::endl;
//     //std::cout << root.stats.visits1[0] << " " << root.stats.visits1[1] << std::endl;
//     // std::cout << "average lock time: " << root.stats.lock_time / (double) threads << std::endl;
//     std::cout << std::endl;

    return 0;
}

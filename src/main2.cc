#include <assert.h>
#include <iostream>

#include "state/toy_state.hh"
#include "model/monte_carlo.hh"
#include "tree/node.hh"
#include "search/exp3p2.hh"

#include <thread>
#include <vector>

    using ToyState1 = ToyState<9>;
    using MoldState1 = MoldState<9>;
    using MonteCarlo1 = MonteCarlo<MoldState1>;
    using Exp3pT = Exp3p<MonteCarlo1>;


    prng device;

    MonteCarlo1 model(device);
    
    Exp3pT session(device);

    MatrixNode<Exp3pT> root;

    // Change State Parameters

    ToyState1 toy(device, 'u', 3, 0);
    MoldState1 mold(device, 7);

void search (MatrixNode<Exp3pT>* root, int playouts, int idx) {
    
    for (int playout = 0; playout < playouts; ++playout) {
        auto state_ = mold;
        session.search(state_, model, root);
        //std::cout << idx << ": " << root->stats.visits << std::endl;
    }

};

int main () {

    toy.transition(0, 0);
    mold.transition(0, 0);
    int playouts = 1000000;
    root.stats.t = playouts;


    const int threads = 1;
    std::thread pool[threads];
    for (int i = 0; i < threads; ++i) {
        pool[i] = std::thread(search, &root, playouts / threads, i);
        // pool[i].join();
    }
    for (int i = 0; i < threads; ++i) {
        // pool[i] = std::thread(search, &root, playouts, i);
        pool[i].join();
    }
    std::cout << root.stats.visits0[0] << ' ' << root.stats.visits0[1] << std::endl;
    std::cout << "exp3p2 threads" << std::endl;

    // for (int i = 0; i < playouts; ++i) {
    //     ToyState1 toy_ = toy;
    //     session.search(toy_, model, &root);
    // }
    // std::cout << root.stats.visits0[0] << ' ' << root.stats.visits0[1] << std::endl;
    // std::cout << "exp3p2" << std::endl;

    return 0;
}
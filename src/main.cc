#include <assert.h>
#include <iostream>

#include "state/toy_state.hh"
#include "model/monte_carlo.hh"
#include "tree/node.hh"
#include "search/exp3p_thread.hh"

#include <thread>
#include <vector>

    using ToyState1 = ToyState<9>;
    using MonteCarlo1 = MonteCarlo<ToyState1>;
    using Exp3pT = Exp3p<MonteCarlo1>;


    prng device;

    MonteCarlo1 model(device);
    
    Exp3pT session(device);

    MatrixNode<Exp3pT> root;

    // Change State Parameters

    ToyState1 toy(device, 'u', 4, 2);

void search (MatrixNode<Exp3pT>* root, int playouts, int idx) {

    for (int playout = 0; playout < playouts; ++playout) {
        auto state_ = toy;
        session.search(state_, model, root);
        //std::cout << idx << ": " << root->stats.visits << std::endl;
    }

};

int main () {

    const int threads = 6;
    int playouts = 1000000000;
    std::thread pool[threads];

    for (int i = 0; i < threads; ++i) {
        pool[i] = std::thread(search, &root, playouts / threads, i);
        // pool[i].join();
    }
    for (int i = 0; i < threads; ++i) {
        // pool[i] = std::thread(search, &root, playouts, i);
        pool[i].join();
    }

    return 0;
}

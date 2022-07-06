
#include <assert.h>
#include <iostream>

#include "state/toy_state.hh"
#include "model/monte_carlo.hh"
#include "tree/node.hh"
#include "search/exp3p_thread.hh"
#include "search/matrix_ucb.hh"



int main () {

    using ToyState = ToyState<9>;
    using MoldState = MoldState<9>;
    using MonteCarlo = MonteCarlo<ToyState>;
    using Exp3p = Exp3p<MonteCarlo>;
    using MatrixUCB = MatrixUCB<MonteCarlo>;

    prng device;
    ToyState toy(device, 'w', 2, 0);
    // toy.transition(0,0);
    // MoldState mold(device, 7);
    // mold.transition(0, 0);
    MatrixNode<MatrixUCB> root;
    MatrixUCB session(device);

    // int threads = 4;
    int playouts = 1000;
    session.search(playouts, toy, &root);

    // std::cout << root.child->stats.cumulative_value0 / root.child->stats.visits << std::endl;
    // std::cout << root.child->next->stats.cumulative_value0 / root.child->next->stats.visits << std::endl;
    // std::cout << root.child->next->next->stats.cumulative_value0 / root.child->next->next->stats.visits << std::endl;
    // std::cout << root.child->next->next->next->stats.cumulative_value0 / root.child->next->next->next->stats.visits << std::endl;

    root.stats.cumulative_payoffs.print();
    root.stats.visits.print();
    // TODO remove this
    (root.stats.cumulative_payoffs + root.stats.visits).print();

    std::cout << root.stats.cumulative_payoffs.rows << std::endl;
    std::cout << root.stats.cumulative_payoffs.cols << std::endl;

    return 0;
}

#include <assert.h>
#include <iostream>

#include "state/toy_state.hh"
#include "model/monte_carlo.hh"
#include "tree/node.hh"
#include "search/exp3p.hh"
#include "search/matrix_ucb.hh"

/*
Example of applying MatrixUCB search
*/

int main()
{

    using MoldState = MoldState<2>;
    using MonteCarlo = MonteCarlo<ToyState<2>>;
    using MatrixUCB = MatrixUCB<MonteCarlo>;

    prng device(0);
    ToyState<2> toy_state(device, sucker_punch, 4, 5);
    MoldState mold_state(device, 100);
    MonteCarlo model(device);
    MatrixUCB session(device);
    session.c_uct = .2;
    session.expl_threshold = .001;
    MatrixNode<MatrixUCB> root;

    int playouts = 800000;
    session.search(playouts, toy_state, model, root);
    std::cout << root.stats.strategy0[0] << ' ' << root.stats.strategy0[1] << std::endl;
    std::cout << root.stats.strategy1[0] << ' ' << root.stats.strategy1[1] << std::endl;

    std::cout << "Playouts: " << playouts << std::endl;
    std::cout << "Size of root tree after search: " << root.count() << std::endl;
    std::cout << "expl hits: " << session.expl_hits << std::endl;
    std::cout << "gambit hits: " << session.gambit_hits << std::endl;

    return 0;
}
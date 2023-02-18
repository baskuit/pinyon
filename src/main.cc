
#include <assert.h>
#include <iostream>

#include "state/toy_state.hh"
#include "model/monte_carlo.hh"
#include "tree/node.hh"
#include "search/exp3p.hh"

/*
Example of applying Exp3p search
*/

int main () {

    using MoldState = MoldState<2>;
    using MonteCarlo = MonteCarlo<MoldState>;
    using Exp3p = Exp3p<MonteCarlo>;
    

    prng device(0);
    ToyState<2> toy_state(device, sucker_punch_win_by, 4, 5);
    MoldState mold_state(device, 100);
    MonteCarlo model(device);
    Exp3p session(device);
    MatrixNode<Exp3p> root;

    int playouts = 1000000;
    session.search(playouts, mold_state, root);
    std::cout << "Playouts: " << playouts << std::endl;
    std::cout << "Size of root tree after search: " << root.count() << std::endl;

    return 0;
}
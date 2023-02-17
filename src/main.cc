
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

    using ToyState = ToyState<2>;
    using MonteCarlo = MonteCarlo<ToyState>;
    using Exp3p = Exp3p<MonteCarlo>;


    // Init prng devices to use in algorithm and in this case state
    prng device;
    ToyState toy_state(device, 'w', 2, 2);
    MonteCarlo model();
    Exp3p session(device);
    MatrixNode<Exp3p> root;

    int playouts = 10000;
    session.search(playouts, toy_state, &root);

    return 0;
}
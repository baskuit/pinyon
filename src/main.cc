
#include <assert.h>
#include <iostream>

#include "state/toy_state.hh"
#include "model/monte_carlo.hh"
#include "tree/node.hh"
#include "search/exp3p.hh"
#include "search/matrix_ucb.hh"

/*
Example of applying Exp3p search
*/

int main()
{

    using MoldState = MoldState<2>;
    using MonteCarlo = MonteCarlo<ToyState<2>>;
    using MatrixUCB = MatrixUCB<MonteCarlo>;

    prng device(0);
    ToyState<2> toy_state(device, sucker_punch_win_by, 2, 1);
    MoldState mold_state(device, 100);
    MonteCarlo model(device);
    MatrixUCB session(device);
    MatrixNode<MatrixUCB> root;

    int playouts = 100000;
    session.search(playouts, toy_state, model, root);
    std::cout << "Playouts: " << playouts << std::endl;
    std::cout << "Size of root tree after search: " << root.count() << std::endl;

    return 0;
}
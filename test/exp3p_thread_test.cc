
#include <assert.h>
#include <iostream>

#include "state/toy_state.hh"
#include "model/monte_carlo.hh"
#include "tree/node.hh"
#include "search/multithreading/exp3p_thread.hh"

/*
Test of Exp3p with multithreading

Note multithreading is generally slower on small trees due to high contention.
Test speed on MoldState or a real game to see benefits.
*/

int main()
{
    using MoldState = MoldState<2>;
    using MonteCarlo = MonteCarlo<ToyState<2>>;
    using Exp3p = Exp3p<MonteCarlo>;

    prng device(0);
    ToyState<2> toy_state(device, sucker_punch_win_by, 2, 1);
    MoldState mold_state(device, 100);
    MonteCarlo model(device);
    Exp3p session(device);
    MatrixNode<Exp3p> root;

    int threads = 4;
    int playouts = 100000;
    session.search(threads, playouts, toy_state, model, root);
    std::cout << "Playouts: " << playouts << std::endl;
    std::cout << "Size of root tree after search: " << root.count() << std::endl;
    std::cout << "Exp3p root visits" << std::endl;
    std::cout << "p0: " << root.stats.visits0[0] << ' ' << root.stats.visits0[1] << std::endl;
    std::cout << "p1: " << root.stats.visits1[0] << ' ' << root.stats.visits1[1] << std::endl;
    std::cout << "Exp3p root matrix" << std::endl;
    session.get_matrix(&root).print();
    return 0;
}
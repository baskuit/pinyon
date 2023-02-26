
#include <assert.h>
#include <iostream>

#include "state/test_states.hh"
#include "model/monte_carlo.hh"
#include "search/exp3p.hh"
#include "tree/node.hh"
// #include "search/matrix_ucb.hh"

/*
Example of applying MatrixUCB search
*/

int main()
{
    using MoldState = MoldState<3>;
    using MonteCarlo = MonteCarlo<MoldState>;
    using Exp3p = Exp3p<MonteCarlo>;

    MoldState state(3);
    prng device(0);
    MonteCarlo model(device);

    std::cout << "is_terminal: " << state.is_terminal << std::endl;
    std::cout << "depth: " << state.depth << std::endl;

    MatrixNode<Exp3p> root;
    Exp3p session(device);
    session.run(
        5,
        state,
        model,
        root
    );

    std::cout << "root count: " << root.count() << std::endl;
}
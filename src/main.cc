
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
    using State = PennyMatching;
    using MonteCarlo = MonteCarlo<State>;
    using Exp3p = Exp3p<MonteCarlo>;

    State state;
    prng device(0);
    MonteCarlo model(device);

    std::cout << "is_terminal: " << state.is_terminal << std::endl;

    MatrixNode<Exp3p> root;
    Exp3p session(device);
    session.run(
        200,
        state,
        model,
        root
    );

    std::cout << "root count: " << root.count() << std::endl;
}
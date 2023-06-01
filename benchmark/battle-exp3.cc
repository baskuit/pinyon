#include <surskit.hh>

#include <iostream>

/*

Intended to test mostly tree traversal speed

*/

int main () {

    using MonteCarlo = MonteCarloModel<Battle>;
    using Exp3 = Exp3<MonteCarlo>;
    
    uint64_t seed = 0;
    prng device(seed);
    Battle state(10);
    MonteCarlo model(device);
    Exp3 session(.01);
    MatrixNode<Exp3> root;
    const size_t iterations = 1000000;
    session.run(iterations, device, state, model, root);

    return 0;
}
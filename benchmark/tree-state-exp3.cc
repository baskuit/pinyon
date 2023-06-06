#include <surskit.hh>

/*

Intended to test mostly tree traversal speed

*/

int main () {

    using MonteCarlo = MonteCarloModel<RandomTree>;
    using Exp3 = Exp3<MonteCarlo>;
    
    uint64_t seed = 0;
    prng device(seed);
    RandomTree state(device, 10, 3, 3, 1, 0);
    MonteCarlo model(device);
    Exp3 session(.01);
    Exp3::Types::MatrixNode root;
    const size_t iterations = 1000000;
    session.run(iterations, device, state, model, root);

    return 0;
}
#include <surskit.hh>

/*

Intended to test mostly tree traversal speed

*/

int main () {

    using MoldState = MoldState<2>;
    using MonteCarlo = MonteCarloModel<MoldState::T>;
    using Exp3 = Exp3<MonteCarlo::T>;
    using Search = TreeBandit<Exp3::T>;
    
    uint64_t seed = 0;
    prng device(seed);
    MoldState state(10);
    MonteCarlo model(device);
    Search session(.01);
    Search::MatrixNode root;
    const size_t iterations = 1000000;
    session.run(iterations, device, state, model, root);

    return 0;
}
#include <surskit.hh>

/*

Intended to test mostly tree traversal speed

*/

int main () {

    using MoldState = MoldState<2>;
    using MonteCarlo = MonteCarloModel<MoldState>;
    using MatrixUCB = MatrixUCB<MonteCarlo>;
    
    uint64_t seed = 0;
    prng device(seed);
    MoldState state(10);
    MonteCarlo model(device);
    MatrixUCB session;
    MatrixNode<MatrixUCB> root;
    const size_t iterations = 10000;
    session.run(iterations, device, state, model, root);

    return 0;
}
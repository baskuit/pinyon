#include "tree_state.hh"

int main () {
    prng device(0);
    SeedState<4> seed(device, 0, 4, 4);
    Grow<MonteCarlo<SeedState<4>>> session(device);
    MatrixNode<Grow<MonteCarlo<SeedState<4>>>>* root;
    session.grow(seed, root);
    return 0;
}
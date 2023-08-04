#include <surskit.hh>

/*

Intended to test mostly tree traversal speed

*/

int main()
{

    using Types = TreeBandit<Exp3<MonteCarloModel<MoldState<2>>>, DebugNodes>;

    uint64_t seed = 0;
    prng device(seed);
    Types::State state(10);
    Types::Model model(device);
    Types::Search session(.01);
    Types::MatrixNode root;
    const size_t iterations = 1 << 20;
    session.run_for_iterations(iterations, device, state, model, root);

    std::cout << "iterations: " << iterations << std::endl;
    std::cout << "matrix node count: " << root.count_matrix_nodes() << std::endl;

    return 0;
}
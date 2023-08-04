#include <surskit.hh>

/*

Intended to test mostly tree traversal speed

*/

int main()
{

    using Types = TreeBandit<Exp3<MonteCarloModel<MoldState<2>>>>;
    using Types_ = TreeBandit<Exp3<MonteCarloModel<MoldState<3>>>>;

    Types::TypeList x{};
    Types_::TypeList y{};
    x = y;

    uint64_t seed = 0;
    prng device(seed);
    Types::State state(1);
    Types::Model model(device);
    Types::Search session(.01);
    Types::MatrixNode root;
    const size_t iterations = 3;
    session.run_for_iterations(iterations, device, state, model, root);

    return 0;
}
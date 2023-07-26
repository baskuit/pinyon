#include <surskit.hh>

using State = Arena<RandomTree<>, MonteCarloModel<RandomTree<>>>;
using Model = MonteCarloModel<State>;
using Algorithm = TreeBanditThreaded<Exp3<Model>>;

W::StateWrapper<RandomTree<>> generator(typename State::Types::Seed seed)
{
    const size_t max_depth_bound = 5;
    const size_t max_actions = 5;
    const size_t max_transitions = 3;

    prng device{seed};

    const size_t depth_bound = device.random_int(max_depth_bound) + 1;
    const size_t actions = device.random_int(max_actions) + 1;
    const size_t transitions = device.random_int(max_transitions) + 1;

    return W::StateWrapper<RandomTree<>>{seed, depth_bound, actions, actions, transitions, Rational<>{0}};
}

int main()
{

    std::vector<W::SearchWrapper<TreeBandit<Exp3<MonteCarloModel<RandomTree<>>>>>> agents = {
        {.01}, {.1}, {1}};

    const size_t iterations = 1 << 10;
    prng device{0};
    W::ModelWrapper<MonteCarloModel<RandomTree<>>> model{device};

    State arena{iterations, &generator, model, agents};

    Model arena_model{1337};
    Algorithm session{.1};
    session.threads = 6;
    MatrixNode<Algorithm> root;

    session.run(1000, device, arena, arena_model, root);
    State::Types::VectorReal row_strategy, col_strategy;
    session.get_empirical_strategies(root.stats, row_strategy, col_strategy);
    math::print(row_strategy);
    math::print(col_strategy);

    return 0;
}
#include <surskit.hh>

// using Types = TreeBanditThreaded<Exp3<MonteCarloModel<Arena<MonteCarloModel<RandomTree<>>>>>>;

// W::StateWrapper<RandomTree<>> generator_function(Types::Seed seed)
// {
//     const size_t max_depth_bound = 5;
//     const size_t max_actions = 5;
//     const size_t max_transitions = 3;

//     prng device{seed};

//     const size_t depth_bound = device.random_int(max_depth_bound) + 1;
//     const size_t actions = device.random_int(max_actions) + 1;
//     const size_t transitions = device.random_int(max_transitions) + 1;

//     return W::StateWrapper<RandomTree<>>{seed, depth_bound, actions, actions, transitions, Rational<>{0}};
// }

int main()
{
    // std::vector<W::Search*> agents = {
    //     {.01}, {.1}, {1}};

    // const size_t iterations = 1 << 10;
    // prng device{0};
    // W::ModelWrapper<Types>::Model model{device};

    // Types::State arena{iterations, &generator_function, model, agents};

    // Types::Model arena_model{1337};
    // Types::Search session{.1};
    // session.threads = 6;
    // Types::MatrixNode root;

    // session.run(1000, device, arena, arena_model, root);
    // Types::VectorReal row_strategy, col_strategy;
    // session.get_empirical_strategies(root.stats, row_strategy, col_strategy);
    // math::print(row_strategy);
    // math::print(col_strategy);

    return 0;
}
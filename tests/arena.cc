#include <surskit.hh>

W::Types::State generator_function(W::Types::Seed seed)
{
    const size_t max_depth_bound = 5;
    const size_t max_actions = 5;
    const size_t max_transitions = 3;

    prng device{seed};

    const size_t depth_bound = device.random_int(max_depth_bound) + 1;
    const size_t actions = device.random_int(max_actions) + 1;
    const size_t transitions = device.random_int(max_transitions) + 1;
    
    return W::make_state<RandomTree<>>(seed, depth_bound, actions, actions, transitions, Rational<>{0});
    // return W::StateWrapper<RandomTree<>>{seed, depth_bound, actions, actions, transitions, Rational<>{0}};
}

int main()
{

    using Types = TreeBandit<Exp3<MonteCarloModel<RandomTree<>>>>;
    using ArenaTypes = TreeBanditThreaded<Exp3<MonteCarloModel<Arena>>>;

    std::vector<W::Types::Search> agents = {
        W::make_search<Types>(.01), 
        W::make_search<Types>(0.1), 
        W::make_search<Types>(1.0)
    };

    const size_t iterations = 1 << 10;
    prng device{0};
    auto model = W::make_model<Types>(device);

    ArenaTypes::State arena{iterations, &generator_function, model, agents};
    ArenaTypes::Model arena_model{1337};
    ArenaTypes::Search search{.1};
    search.threads = 6;
    ArenaTypes::MatrixNode root;

    search.run_for_iterations(12, device, arena, arena_model, root);
    ArenaTypes::VectorReal row_strategy, col_strategy;
    search.get_empirical_strategies(root.stats, row_strategy, col_strategy);
    math::print(row_strategy);
    math::print(col_strategy);

    return 0;
}
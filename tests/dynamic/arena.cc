#include <surskit.hh>

#include <algorithm/tree-bandit/bandit/exp3-fat.hh>

using SolvedStateTypes = TraversedState<EmptyModel<RandomTree<>>>;

W::Types::State generator_function(const W::Types::Seed seed)
{
    // const size_t max_depth_bound = 5;
    // const size_t max_actions = 3;
    // const size_t max_transitions = 1;

    // prng device{seed};

    const size_t depth_bound = 1; // device.random_int(max_depth_bound) + 1;
    const size_t actions = 5;     // device.random_int(max_actions) + 2;
    const size_t transitions = 1; // device.random_int(max_transitions) + 1;

    RandomTree<>::State random_tree{seed, depth_bound, actions, actions, transitions, Rational<>{0}};
    EmptyModel<RandomTree<>>::Model random_tree_model{};
    SolvedStateTypes::State solved_random_tree{random_tree, random_tree_model};

    return W::make_state<SolvedStateTypes>(solved_random_tree);
}

int main()
{

    using MCTypes = TreeBandit<Exp3<MonteCarloModel<SolvedStateTypes>>>;
    // Exp3 search types on a solved random tree
    using MCTypesModel = TreeBanditSearchModel<MCTypes>;
    // Model type that treats search output as its inference
    using ArenaTypes = TreeBandit<Exp3Fat<MonteCarloModel<Arena>>>;
    // Type list for multithreaded exp3 over Arena state

    const size_t monte_carlo_iterations = 1 << 10;

    EmptyModel<SolvedStateTypes>::Model
        model_r{};
    MCTypesModel::Model
        model_b{monte_carlo_iterations, MCTypes::PRNG{0}, MCTypes::Model{0}, MCTypes::Search{.10}};
    SolvedStateModel<SolvedStateTypes>::Model
        model_s{};

    std::vector<W::Types::Model> models{
        // W::make_model<Rand<EmptyModel<SolvedStateTypes>>>(model_r),
        W::make_model<SolvedStateModel<SolvedStateTypes>>(model_s),
        W::make_model<MCTypesModel>(model_b)};

    const size_t threads = 4;
    ArenaTypes::PRNG device{0};
    ArenaTypes::State arena{&generator_function, models};
    ArenaTypes::Model arena_model{1337};
    ArenaTypes::Search search{ArenaTypes::BanditAlgorithm{.01}};
    ArenaTypes::MatrixNode root;

    const size_t arena_search_iterations = 1 << 14;
    search.run_for_iterations(arena_search_iterations, device, arena, arena_model, root);

    root.stats.matrix.print();

    return 0;
}

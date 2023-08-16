#include <surskit.hh>

#include <algorithm/tree-bandit/bandit/exp3-fat.hh>

// using UnsolvedStateTypes = OneSumMatrixGame;
using UnsolvedStateTypes = RandomTree<>;
using SolvedStateTypes = TraversedState<EmptyModel<UnsolvedStateTypes>>;
using FinalStateTypes = UnsolvedStateTypes;

prng a_device{};

W::Types::State generator_function(const W::Types::Seed seed)
{
    const size_t depth_bound = 12;
    const size_t actions = 3;
    const size_t transitions = 1;

    UnsolvedStateTypes::State random_tree{a_device.uniform_64(), depth_bound, actions, actions, transitions, Rational<>{0}};
    // EmptyModel<UnsolvedStateTypes>::Model random_tree_model{};
    // SolvedStateTypes::State solved_random_tree{random_tree, random_tree_model};

    return W::make_state<FinalStateTypes>(random_tree);
}

int main()
{

    using MCTypes = TreeBandit<Exp3<MonteCarloModel<FinalStateTypes>>>;
    // Exp3 search types on a solved random tree
    using MCTypesModel = TreeBanditSearchModel<MCTypes>;
    // Model type that treats search output as its inference
    using ArenaTypes = TreeBanditThreaded<Exp3Fat<MonteCarloModel<Arena>>>;
    // Type list for multithreaded exp3 over Arena state

    const size_t monte_carlo_iterations = 1 << 10;

    EmptyModel<SolvedStateTypes>::Model
        model_random{};
    SolvedStateModel<SolvedStateTypes>::Model
        model_solved{};

    // vector of search-model. initializer is iteration, device, model, search.
    std::vector<MCTypesModel::Model> modelo{
        {monte_carlo_iterations, MCTypes::PRNG{0}, MCTypes::Model{0}, MCTypes::Search{.01}},
        {monte_carlo_iterations, MCTypes::PRNG{0}, MCTypes::Model{0}, MCTypes::Search{.05}},
        {monte_carlo_iterations, MCTypes::PRNG{0}, MCTypes::Model{0}, MCTypes::Search{.10}},
        {monte_carlo_iterations, MCTypes::PRNG{0}, MCTypes::Model{0}, MCTypes::Search{.20}},
        {monte_carlo_iterations, MCTypes::PRNG{0}, MCTypes::Model{0}, MCTypes::Search{.5}},
        {monte_carlo_iterations, MCTypes::PRNG{0}, MCTypes::Model{0}, MCTypes::Search{1}},
    };
    
    std::vector<W::Types::Model> models;// = {W::make_model<EmptyModel<SolvedStateTypes>>(model_random)};

    std::transform(
        modelo.cbegin(), modelo.cend(), std::back_inserter(models),
        [](const MCTypesModel::Model &m)
        { return W::make_model<MCTypesModel>(m); });

    const size_t threads = 4;
    ArenaTypes::PRNG device{0};
    const size_t vs_rounds = 16;
    ArenaTypes::State arena{&generator_function, models, vs_rounds};
    ArenaTypes::Model arena_model{1337};
    ArenaTypes::Search search{ArenaTypes::BanditAlgorithm{.10}, threads};
    ArenaTypes::MatrixNode root;

    const size_t arena_search_iterations = 1 << 16;
    search.run_for_iterations(arena_search_iterations, device, arena, arena_model, root);

    root.stats.matrix.print();
    std::cout << a_device.get_seed() << std::endl;

    return 0;
}

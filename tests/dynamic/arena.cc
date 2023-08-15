#include <surskit.hh>

#include <algorithm/tree-bandit/bandit/exp3-fat.hh>

// using UnsolvedStateTypes = OneSumMatrixGame;
using UnsolvedStateTypes = RandomTree<>;
using SolvedStateTypes = TraversedState<EmptyModel<UnsolvedStateTypes>>;

prng a_device{};

W::Types::State generator_function(const W::Types::Seed seed)
{
    const size_t depth_bound = 4;
    const size_t actions = 3;
    const size_t transitions = 1;

    UnsolvedStateTypes::State random_tree{a_device.uniform_64(), depth_bound, actions, actions, transitions, Rational<>{0}};
    EmptyModel<UnsolvedStateTypes>::Model random_tree_model{};
    SolvedStateTypes::State solved_random_tree{random_tree, random_tree_model};

    return W::make_state<SolvedStateTypes>(solved_random_tree);
}

int main()
{

    using MCTypes = TreeBandit<Exp3<MonteCarloModel<SolvedStateTypes>>>;
    // Exp3 search types on a solved random tree
    using MCTypesModel = TreeBanditSearchModel<MCTypes>;
    // Model type that treats search output as its inference
    using ArenaTypes = TreeBanditThreaded<Exp3Fat<MonteCarloModel<Arena>>>;
    // Type list for multithreaded exp3 over Arena state

    const size_t monte_carlo_iterations = 1 << 10;

    EmptyModel<SolvedStateTypes>::Model
        model_random{};
    MCTypesModel::Model
        model_a{monte_carlo_iterations, MCTypes::PRNG{0}, MCTypes::Model{0}, MCTypes::Search{.01}};
    MCTypesModel::Model
        model_b{monte_carlo_iterations, MCTypes::PRNG{0}, MCTypes::Model{0}, MCTypes::Search{.10}};
    MCTypesModel::Model
        model_c{monte_carlo_iterations, MCTypes::PRNG{0}, MCTypes::Model{0}, MCTypes::Search{.25}};
    SolvedStateModel<SolvedStateTypes>::Model
        model_solved{};

    std::vector<W::Types::Model>
        models{
            W::make_model<Rand<EmptyModel<SolvedStateTypes>>>(model_random),
            W::make_model<MCTypesModel>(model_a),
            W::make_model<MCTypesModel>(model_b),
            W::make_model<MCTypesModel>(model_c),
            W::make_model<SolvedStateModel<SolvedStateTypes>>(model_solved)};

    const size_t threads = 4;
    ArenaTypes::PRNG device{0};
    ArenaTypes::State arena{&generator_function, models};
    ArenaTypes::Model arena_model{1337};
    ArenaTypes::Search search{ArenaTypes::BanditAlgorithm{.10}, threads};
    ArenaTypes::MatrixNode root;

    const size_t arena_search_iterations = 1 << 17;
    search.run_for_iterations(arena_search_iterations, device, arena, arena_model, root);

    root.stats.matrix.print();
    std::cout << a_device.get_seed() << std::endl;

    return 0;
}

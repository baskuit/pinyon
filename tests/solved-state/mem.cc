#include <surskit.hh>

// using UnsolvedStateTypes = MoldState<2>;
using UnsolvedStateTypes = RandomTree<RandomTreeRationalTypes>;
using SolvedStateTypes = TraversedState<EmptyModel<UnsolvedStateTypes>>;

using FT = FullTraversal<EmptyModel<UnsolvedStateTypes>>;
using FT_ = TreeBandit<Exp3<EmptyModel<UnsolvedStateTypes>>>;

prng a_device{};

auto generator_function(const W::Types::Seed seed)
{
    const size_t depth_bound = 4;
    const size_t actions = 3;
    const size_t transitions = 1;

    // UnsolvedStateTypes::State state{2};
    UnsolvedStateTypes::State state{0, 3, 4, 4, 1};
    EmptyModel<UnsolvedStateTypes>::Model model{};
    SolvedStateTypes::State solved_state{state, model};

    return W::make_state<UnsolvedStateTypes>(solved_state);
}

int main()
{
    for (size_t i = 0; i < (1 << 30); ++i)
    {
        //     auto state = generator_function(0);
        // }
        const size_t depth_bound = 4;
        const size_t actions = 3;
        const size_t transitions = 1;
        FT_::Search solver{};
        UnsolvedStateTypes::State state{prng{}, 3, 4, 4, 1};
        FT_::Model model{};
        FT_::MatrixNode root{};
        // auto cn = root.access(0, 0);
        // cn->access(state.obs);
        // solver.run(state, model, &root);
        FT_::PRNG device{0};
        solver.run_for_iterations(100, device, state, model, root);
        std::cout << root.count_matrix_nodes() << std::endl;
        if (root.count_matrix_nodes() != 100) {
            exit(1);
        }
    }

    return 0;
}

#include <pinyon.hh>

// using UnsolvedStateTypes = MoldState<2>;
using UnsolvedStateTypes = RandomTree<>;
using SolvedStateTypes = TraversedState<EmptyModel<UnsolvedStateTypes>>;

using FT = FullTraversal<EmptyModel<UnsolvedStateTypes>>;
// using FT = TreeBandit<Exp3<EmptyModel<UnsolvedStateTypes>>>;

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

        const size_t depth_bound = 4;
        const size_t actions = 3;
        const size_t transitions = 1;
        FT::Search solver{};
        UnsolvedStateTypes::State state{0, 3, 4, 4, 1};
        FT::Model model{};
        FT::MatrixNode root{};
        solver.run(state, model, root);
        
        // FT_::PRNG device{0};
        // solver.run_for_iterations(100, device, state, model, root);

    }

    return 0;
}

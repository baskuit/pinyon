#include "model/model.hh" //TODO this must be first otherwise include error (algo.hh wants model.hh...) fix with cmake!!!
#include "seed_state.hh"
#include "grow.hh"
#include "tree_state.hh"

// #include "search/matrix_ucb.hh"

#include <iostream>

int main()
{

    const int size = 3;

    using SeedState = SeedState<size>;
    using MonteCarlo = MonteCarloModel<SeedState>;
    using Grow = Grow<MonteCarlo>;
    using TreeState = TreeState<size>;
    // Initialization now runs the Grow algorithm automatically
    prng device;
    TreeState tree_state(device, 2, size, size);
    std::cout << "tree count: " << tree_state.current->count() << std::endl;
    std::cout << "root expected value matrix" << std::endl;
    tree_state.current->stats.expected_value.print();
    std::cout << "strategies" << std::endl;
    math::print(tree_state.row_strategy, size);
    math::print(tree_state.col_strategy, size);

    return 0;
}
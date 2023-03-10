#include "model/model.hh" //TODO this must be first otherwise include error (algo.hh wants model.hh...) fix with cmake!!!
#include "../random_trees/seed_state.hh"
#include "../random_trees/grow.hh"
#include "../random_trees/tree_state.hh"

#include "search/matrix_ucb.hh"
#include "search/exp3p.hh"

#include <iostream>

const int __size__ = 3;

int main()
{

    using TreeState = TreeState<__size__>;
    using MonteCarlo = MonteCarloModel<TreeState>;

    prng device;
    TreeState tree_state(device, 2, __size__, __size__);
    // Initialization now runs the Grow algorithm automatically

    std::cout << "tree count: " << tree_state.current->count() << std::endl;
    std::cout << "root expected value matrix" << std::endl;
    tree_state.current->stats.expected_value.print();
    std::cout << "strategies" << std::endl;
    math::print(tree_state.row_strategy, __size__);
    math::print(tree_state.col_strategy, __size__);

    return 0;
}
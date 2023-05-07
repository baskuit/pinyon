#include "seed-state.hh"
#include "model/model.hh"
#include "algorithm/exp3p.hh"
#include "algorithm/matrix-ucb.hh"

#include "grow.hh"

#include <iostream>

int main()
{

    const int MaxActions = 3;
    const int MaxTransitions = 1;

    using SeedState = SeedState<MaxActions, MaxTransitions>;
    using Model = MonteCarloModel<SeedState>;
    using Algorithm = Exp3p<Model, TreeBandit>;

    prng device(1);

    SeedState state(device, 1, 3, 3, nullptr, nullptr, nullptr);
    Model model(device);
    Algorithm session;
    MatrixNode<Algorithm> root;

    session.run(100000, device, state, model, root);

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            auto child = root.access(i, j)->child;
            std::cout << child->inference.row_value << ' ';
        }
        std::cout << '\n';
    }
    Algorithm::Types::VectorReal r(3), s(3);
    session.get_strategies(&root, r, s);
    math::print(r, 3);
    math::print(s, 3);

    /*
    Bug cus no call get_actions to get transition_strategies
    Where should we put the get_actions call to guarantee no search is done on state that doesnt have actions?
    If put at run, then
    */

    // Grow<Model> session2;
    // MatrixNode<Grow<Model>> root2;
    // session2.grow(state, model, &root2);
    // root2.stats.nash_payoff_matrix.print();
    const int x = root.count_matrix_nodes();
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            auto state_copy = state;
            // state_copy.get_actions();
            state_copy.apply_actions(i, j);
            std::cout << state_copy.row_payoff << ' ';
        }
        std::cout << '\n';
    } // Print payoff of root stage.

    return 0;
}
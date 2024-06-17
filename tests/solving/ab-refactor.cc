#include <pinyon.hh>

int main() {
    using T = MonteCarloModel<RandomTree<RandomTreeRationalTypes>>;
    using Types = AlphaBetaRefactor<T, false>;
    using Solver = FullTraversal<T>;

    Types::PRNG device{2341343};

    const size_t tree_depth = 2;
    const size_t transitions = 1;

    for (int actions = 2; actions <= 9; ++actions) {
        for (int trials = 0; trials < 10; ++trials) {
            const uint64_t seed = device.uniform_64();
            const Types::State state{prng{seed}, tree_depth, actions, actions, transitions};
            auto state_copy = state;
            state_copy.apply_actions(0,0);
            std::cout << "state is terminal 0" << state_copy.is_terminal() << std::endl;
            state_copy.apply_actions(0,0);
            std::cout << "state is terminal 1" << state_copy.is_terminal() << std::endl;
            std::cout << "SEED: " << seed << std::endl;
            Types::Model model{prng{345357457}};
            Types::Search search{};
            Types::MatrixNode node{};

            Solver::Search solver{};
            Solver::MatrixNode solver_node{};

            std::vector<size_t> singleton{};
            singleton.push_back(tree_depth);
            auto output = search.run(singleton, device, state, model, node);
            solver.run(tree_depth, device, state, model, solver_node);

            std::cout << "payoff: " << solver_node.stats.payoff.get_row_value().get_str() << std::endl;
            std::cout << "alpha: " << output.alpha.get_str() << " beta: " << output.beta.get_str() << std::endl;

            solver_node.stats.nash_payoff_matrix.print();
            mpq_class x = solver_node.stats.payoff.get_row_value();
            math::canonicalize(x);
            assert(x == output.alpha);
            assert(x == output.beta);
        }
    }
}
#include <pinyon.hh>

int main() {
    debug_print();

    using T = MonteCarloModel<RandomTree<RandomTreeRationalTypes>>;
    using Types = AlphaBetaRefactor<T>;
    using Solver = FullTraversal<T>;

    Types::PRNG device{2341343};

    for (int trials = 0; trials < 1; ++trials) {
        const uint64_t seed = 17906215329761056373;//device.uniform_64();
        Types::State state{prng{seed}, 1, 3, 3, 1};
        std::cout << "SEED: " << seed << std::endl;
        Types::Model model{prng{345357457}};
        Types::Search search{};
        Types::MatrixNode node{};

        Solver::Search solver{};
        Solver::MatrixNode solver_node{};

        auto output = search.run(1, device, state, model, node);
        solver.run(1, device, state, model, solver_node);
        std::cout << "payoff: " << solver_node.stats.payoff.get_row_value().get_str() << std::endl;
        std::cout << "alpha: " << output.alpha.get_str() << " beta: " << output.beta.get_str() << std::endl;

        solver_node.stats.nash_payoff_matrix.print();
    }

}
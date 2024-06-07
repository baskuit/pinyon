#include <pinyon.hh>

int main () {
    using T = MonteCarloModel<RandomTree<RandomTreeRationalTypes>>;
    using Types = AlphaBetaRefactor<T>;
    using Solver = FullTraversal<T>;

    Types::PRNG device{2341343};
    Types::State state{prng{1235643}, 1, 2, 2, 1};
    Types::Model model{prng{345357457}};
    Types::Search search{};
    Types::MatrixNode node{};

    Solver::Search solver{};
    Solver::MatrixNode solver_node{};    

    auto output = search.run(1, device, state, model, node);
    solver.run(1, device, state, model, solver_node);
    std::cout << "payoff: " << solver_node.stats.payoff.get_row_value().get_str() << std::endl;

    solver_node.stats.nash_payoff_matrix.print();
}
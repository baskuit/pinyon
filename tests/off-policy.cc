#include <surskit.hh>
// using Types = OffPolicy<Exp3<TorchModel<>>>;
using Types = OffPolicy<Exp3<MonteCarloModel<RandomTree<>>>>;
using SolvedTypes = TraversedState<MonteCarloModel<RandomTree<>>>;

int main () {
    prng device{0};
    Types::State state{device, 3, 3, 3, 1};
    Types::Model model{0};
    SolvedTypes::State solved_state{state, model};
    Types::Search session{.1};

    std::vector<Types::State> states{state};
    std::vector<Types::MatrixNode> matrix_nodes{{}};

    const size_t total_iterations = 1 << 13;
    const size_t actor_iterations = 1 << 3;
    const size_t learner_iterations = total_iterations / actor_iterations;

    session.run(
        learner_iterations,
        actor_iterations,
        device,
        states,
        model,
        matrix_nodes);

    Types::VectorReal r, c;

    session.get_empirical_strategies(
        matrix_nodes[0].stats, r, c
    );

    std::cout << "off policy strategies:" << std::endl;
    math::print(r);
    math::print(c);

    Types::MatrixValue payoff_matrix;
    solved_state.get_matrix(payoff_matrix);
    std::cout << "matrix:" << std::endl;
    payoff_matrix.print();

    solved_state.get_strategies(r, c);
    std::cout << "NE strategies:" << std::endl;
    math::print(r);
    math::print(c);


    return 0;
}
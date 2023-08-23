#include <surskit.hh>

using Types = OffPolicy<Exp3<MonteCarloModel<MoldState<2>>>>;

int main () {

    Types::State state{size_t{10}};
    Types::Model model{0};
    Types::Search session{};

    std::vector<Types::State> states{state};
    std::vector<Types::MatrixNode> matrix_nodes{{}};
    prng device{0};

    session.run(
        1,
        1 << 3,
        device,
        states,
        model,
        matrix_nodes);


    return 0;
}
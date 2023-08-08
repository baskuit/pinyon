#include <surskit.hh>

using Types = TreeBandit<Exp3<MonteCarloModel<MoldState<2>>>>;

int main() {
    W::Types::State state{Types{}, size_t{10}};
    W::Types::Model model{Types{}, uint64_t{}};
    W::Types::Search search{Types{}, typename Types::Search{}};
    W::Types::MatrixNode matrix_node{Types{}};

    // W::Types::ModelOutput output;
    // model.get_inference(state, output);

    W::Types::PRNG device{};

    size_t iterations = search.run(
        10,
        device,
        state,
        model,
        matrix_node
    );

    std::cout << iterations << std::endl;

}
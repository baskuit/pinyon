#include <surskit.hh>

using Types = TreeBandit<Exp3<MonteCarloModel<MoldState<2>>>>;


int main() {
    // using W::Types::State;

    auto state = W::make_state<Types>(10);
    auto model = W::make_model<Types>(Types::Model{});
    auto search = W::make_search<Types>(Types::Search{});
    W::Types::MatrixNode matrix_node{Types{}};
    W::Types::ModelOutput inference;
    W::Types::PRNG device{};
    // model.inference(state, inference);
    W::Types::VectorReal row_strategy, col_strategy;
    search.run_for_iterations(1000, device, state, model, matrix_node);
    search.get_strategies(matrix_node, row_strategy, col_strategy);
    
    
    // W::Types::Search search{Types{}, typename Types::Search{}};
    // W::Types::MatrixNode matrix_node{Types{}};

    // // W::Types::ModelOutput output;
    // // model.inference(state, output);

    // W::Types::PRNG device{};

    // size_t iterations = search.run(
    //     10,
    //     device,
    //     state,
    //     model,
    //     matrix_node
    // );

    // std::cout << iterations << std::endl;

}
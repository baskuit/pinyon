#include <pinyon.hh>

int main () {
    using T = MonteCarloModel<RandomTree<RandomTreeRationalTypes>>;
    using Types = AlphaBetaRefactor<T>;

    Types::PRNG device{2341343};
    Types::State state{prng{1235643}, 1, 3, 3, 1};
    Types::Model model{prng{345357457}};
    Types::Search search{};
    Types::MatrixNode node{};
    auto output = search.run(1, device, state, model, node);
    1 + 1;
}
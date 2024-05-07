#include <pinyon.hh>

int main () {
    using T = MonteCarloModel<RandomTree<RandomTreeRationalTypes>>;
    using Types = AlphaBetaRefactor<T>;

    Types::State state{prng{1235643}, 1, 3, 3, 1};
    Types::Model model{prng{345357457}};
    Types::Search search{};
}
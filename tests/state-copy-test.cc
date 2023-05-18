#include "../src/state/random-tree.hh"
#include "../src/model/model.hh"
#include "../src/algorithm/bandits/exp3.hh"
#include "../src/algorithm/solve/full-traversal.hh"
#include "../src/algorithm/solve/alphabeta.hh"


const size_t MaxTransitions = 1;

void foo () {
    using State = RandomTree<1>;
    using Model = MonteCarloModel<State>;
    using Exp3 = Exp3<Model, TreeBandit>;

    prng device(0);
    State state(device, 1, 2, 2, 0);
    Model model(device);
    Exp3 session;

    MatrixNode<Exp3> root;
    session.run(1000000, device, state, model, root);

    MatrixNode<FullTraversal<Model>> root_;
    FullTraversal<Model> session_;
    session_.run(state, model, &root_);
}

int main () {

    foo();

    return 0;
}
#include "state/test_states.hh"
#include "model/model.hh"
#include "algorithm/multithreaded.hh"
#include "algorithm/exp3p.hh"
#include "algorithm/matrix_ucb.hh"
#include <iostream>

const int size = 2;

int main()
{
    using MoldState = MoldState<size>;
    using Model = MonteCarloModel<MoldState>;
    using MatrixUCB = MatrixUCB<Model, TreeBanditThreaded>;

    MoldState game(10);
    prng device;
    Model model(device);
    MatrixNode<MatrixUCB> root;
    MatrixUCB session(device);
    session.run(100, game, model, root);

    math::print(root.stats.row_strategy, size);

    // math::print(root.stats.row_visits, 2);

    return 0;
}
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
    using Exp3p = Exp3p<Model, TreeBanditThreaded>;
    using MatrixUCB = MatrixUCB<Model, TreeBanditThreaded>;

    const int depth = 10;
    MoldState game(depth);
    prng device(0);
    Model model(device);
    MatrixNode<MatrixUCB> root;
    MatrixUCB session(device);
    session.threads = 1;
    const int playouts = 1000000;

    std::cout << "Threaded speed test with threads=" << session.threads << ", playouts = " << playouts << ", depth = " << depth << std::endl;

    session.run(playouts, device, game, model, root);

    typename Model::Types::VectorReal row_strategy, col_strategy;
    session.get_strategies(&root, row_strategy, col_strategy);
    math::print(row_strategy, size);
    math::print(col_strategy, size);

    return 0;
}
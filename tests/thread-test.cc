#include "state/test-states.hh"
#include "model/model.hh"
#include "algorithm/multithreaded.hh"
#include "algorithm/exp3p.hh"
#include "algorithm/matrix-ucb.hh"
#include <iostream>

const int size = 2;

int main()
{
    using MoldState = MoldState<size>;
    using Model = MonteCarloModel<MoldState>;
    using Exp3p = Exp3p<Model, TreeBanditThreaded>;
    using MatrixUCB = MatrixUCB<Model, TreeBanditThreaded>;
    // 18.25, 18.43 old array
    // 17.44, 17.69 new array forgot model change
    // 17.55, 17.65 new array with model change
    // 24.47, 23.80 vector
    const int depth = 20;
    MoldState game(depth);
    prng device(0);
    Model model(device);
    MatrixNode<Exp3p> root;
    Exp3p session;
    session.threads = 4;
    const int playouts = 10000000;
    // const int playouts = 10;
    std::cout << "Threaded speed test with threads=" << session.threads << ", playouts = " << playouts << ", depth = " << depth << std::endl;

    session.run(playouts, device, game, model, root);

    typename Model::Types::VectorReal row_strategy(size);
    typename Model::Types::VectorReal col_strategy(size);
    session.get_strategies(&root, row_strategy, col_strategy);
    math::print(row_strategy, size);
    math::print(col_strategy, size);

    return 0;
}
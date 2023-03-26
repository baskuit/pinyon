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

    MoldState game(20);
    prng device(0);
    Model model(device);
    MatrixNode<Exp3p> root;
    Exp3p session(device);
    session.threads = 8;
    const int playouts = 10000000;

    std::cout << "Threaded speed test with threads=" << session.threads << ", playouts = " << playouts << std::endl;

    session.run(playouts, device, game, model, root);

    typename Exp3p::Types::VectorReal row_strategy, col_strategy;
    session.get_strategies(&root, row_strategy, col_strategy);
    math::print(row_strategy, size);
    math::print(col_strategy, size);

    return 0;
}
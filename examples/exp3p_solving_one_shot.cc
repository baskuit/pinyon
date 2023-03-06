#include "state/test_states.hh"
#include "model/model.hh"
#include "search/exp3p.hh"
#include "tree/node.hh"

#include <iostream>

const int __size__ = 3;

/*
Using Exp3p on a one shot matrix game to compute Nash Equilibrium
*/

int main()
{

    prng device;

    using State = StateArray<__size__, int, int, int>;
    using MatrixGame = OneShotOneSum<State>;
    using MonteCarloModel = MonteCarloModel<MatrixGame>;
    using Exp3p = Exp3p<MonteCarloModel, TreeBandit>;

    typename MatrixGame::Types::MatrixReal matrix;
    matrix.rows = __size__;
    matrix.cols = __size__;
    // TODO fix initializer

    for (int row_idx = 0; row_idx < __size__; ++row_idx)
    {
        for (int col_idx = 0; col_idx < __size__; ++col_idx)
        {
            matrix(row_idx, col_idx) = device.uniform();
        }
    }
    // randomly initialize matrix

    MatrixGame game(matrix);
    // define game which wraps reference to matrix. very safe!
    MonteCarloModel model(device);
    Exp3p session(device);
    MatrixNode<Exp3p> root;
    session.run(
        1000,
        game,
        model,
        root);
    // standard search procedure

    typename MatrixGame::Types::VectorReal row_strategy;
    typename MatrixGame::Types::VectorReal col_strategy;
    // exp3p tracks answer with integer vector of empirical strategy. so we define a real vector to as output vector for normalizing.

    math::power_norm<typename MatrixGame::Types::VectorInt, typename MatrixGame::Types::VectorReal>(root.stats.row_visits, matrix.rows, 1, row_strategy);
    math::power_norm<typename MatrixGame::Types::VectorInt, typename MatrixGame::Types::VectorReal>(root.stats.col_visits, matrix.cols, 1, col_strategy);

    std::cout << "Matrix:" << std::endl;
    matrix.print();
    std::cout << "row player solution:" << std::endl;
    math::print(row_strategy, __size__);
    std::cout << "col player solution:" << std::endl;
    math::print(col_strategy, __size__);
    // print matrix and solution

    return 0;
}
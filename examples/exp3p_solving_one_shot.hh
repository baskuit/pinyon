#include "state/test_states.hh"
#include "model/model.hh"
#include "algorithm/exp3p.hh"
#include "tree/tree.hh"
#include "../random_trees/tree_state.hh"
#include <iostream>

const int __size__ = 3;

/*
Using Exp3p on a one shot matrix game to compute Nash Equilibrium
*/

int main()
{

    prng device(0);

    using State = StateArray<__size__, int, int, int>;
    using MatrixGame = TreeState<__size__>;
    using MonteCarloModel = MonteCarloModel<MatrixGame>;
    using Exp3p = Exp3p<MonteCarloModel, TreeBandit>;

    // typename MatrixGame::Types::MatrixReal matrix;
    // matrix.rows = __size__;
    // matrix.cols = __size__;
    // // TODO fix initializer

    // for (int row_idx = 0; row_idx < __size__; ++row_idx)
    // {
    //     for (int col_idx = 0; col_idx < __size__; ++col_idx)
    //     {
    //         matrix(row_idx, col_idx) = device.uniform();
    //     }
    // }
    // // randomly initialize matrix

    MatrixGame game(device, 2, __size__, __size__);
    
    // define game which wraps reference to matrix. very safe!
    MonteCarloModel model(device);
    Exp3p session(device);
    MatrixNode<Exp3p> root;
    session.run(
        3000,
        game,
        model,
        root);
    // standard search procedure

    typename MatrixGame::Types::VectorReal row_strategy;
    typename MatrixGame::Types::VectorReal col_strategy;
    session.get_strategies(&root, row_strategy, col_strategy);

    math::print(row_strategy, __size__);
    math::print(col_strategy, __size__);
    math::print(game.row_strategy, __size__);
    math::print(game.col_strategy, __size__);


    return 0;
}
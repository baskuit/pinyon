#include "state/state.hh"
#include "state/test_states.hh"
#include "model/model.hh"
#include "algorithm/exp3p.hh"
#include "algorithm/matrix_ucb.hh"

#include <iostream>
#include "tree/tree.hh"

template <int size>
using SimpleTypes = TypeList<
    int,
    int,
    double,
    double,
    std::array<int, size>,
    std::array<double, size>,
    std::array<int, size>,
    Linear::Matrix<double, size>,
    Linear::Matrix<int, size>>;

int main()
{
    using SimpleTypes = SimpleTypes<2>;

    using Model = MonteCarloModel<Sucker>;
    using MatrixUCB = MatrixUCB<Model, TreeBandit>;

    Sucker game;
    prng device;
    Model model(device);

    MatrixNode<MatrixUCB> root;

    MatrixUCB session(device);

    session.run(
        10, game, model, root);

    root.stats.row_value_matrix.print();
    root.stats.visit_matrix.print();

    math::print(root.stats.row_strategy, 2);
    math::print(root.stats.col_strategy, 2);

    return 0;
}
#include "state/test_states.hh"
#include "model/model.hh"
#include "algorithm/exp3p.hh"
#include "algorithm/matrix_ucb.hh"
#include <iostream>

const int __size__ = 2;

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
    using MoldState = MoldState<__size__>;
    using Model = MonteCarloModel<MoldState>;
    using MatrixUCB = MatrixUCB<Model, TreeBandit>;

    MoldState game(10);
    prng device;
    Model model(device);
    MatrixNode<MatrixUCB> root;
    MatrixUCB session(device);
    session.run(100, game, model, root);

    math::print(root.stats.row_strategy, __size__);

    // math::print(root.stats.row_visits, 2);

    return 0;
}
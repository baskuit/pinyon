#include "state/test_states.hh"
#include "model/model.hh"
#include "search/algorithm2.hh"

#include <iostream>

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
    using SimpleTypes = SimpleTypes<4>;
    using State = State<SimpleTypes>;
    using Model = MonteCarloModel<Sucker>;

    Sucker game;
    prng device;
    Model model(device);
    MatrixNode<Exp3p<Model, TreeBandit>> root;
    Exp3p<Model, TreeBandit> session(device);
    session.run(1000000, game, model, root);

    math::print(root.stats.row_visits, 2);

    return 0;
}
#include "state/state.hh"
#include "state/test_states.hh"
#include "model/model.hh"
#include "search/exp3p.hh"
#include "search/matrix_ucb.hh"

#include <iostream>
#include "tree/node.hh"

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
    // using State = State<SimpleTypes>;
    // using MoldState = MoldState<4>;
    using Model = MonteCarloModel<Sucker>;
    using MatrixUCB = MatrixUCB<Model, TreeBandit>;

    // // MoldState mold_state(3);
    Sucker game;
    prng device;
    Model model(device);

    MatrixNode<MatrixUCB> root;

    MatrixUCB session(device);
    
    session.run(
        10, game, model, root
    );

    root.stats.row_value_matrix.print();
    root.stats.visit_matrix.print();

    math::print(root.stats.row_strategy, 2);
    math::print(root.stats.row_strategy, 2);

    return 0;
}
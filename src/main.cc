#include "state/state.hh"
#include "state/test_states.hh"
#include "model/model.hh"
#include "search/algorithm.hh"
#include "search/exp3p.hh"

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
    using SimpleTypes = SimpleTypes<4>;
    using State = State<SimpleTypes>;
    using MoldState = MoldState<4>;
    using Model = MonteCarloModel<Sucker>;
    using Exp3p = Exp3p<Model>;
    using TreeBandit = TreeBandit<Exp3p>;

    // MoldState mold_state(3);
    Sucker game;
    prng device;
    Model model(device);

    MatrixNode<Exp3p> root;
    /*
    TreeBandit will derive to TreeBanditMultiThreaded(Pool) so we want to accept Algorithm as template param
    */

    TreeBandit session(device);

    // Linear::Matrix<double, 5> x(1, 3, 3);
    // (x*x).print();
    
    session.run(
        1000000, game, model, root
    );

    math::print(root.stats.row_visits, 2);
    math::print(session.row_forecast, 2);

    return 0;
}
#include "state/state.hh"
#include "state/test_states.hh"
#include "model/model.hh"
#include "search/algorithm.hh"
#include "search/e.hh"

#include <iostream>
#include "tree/node.hh"

template <int size>
using SimpleTypes = TypeList<int, int, double, double, std::array<int, size>, std::array<double, size>, std::array<int, size>>;

int main()
{
    using SimpleTypes = SimpleTypes<4>;
    using State = State<SimpleTypes>;
    using MoldState = MoldState<4>;
    using Model = MonteCarloModel<PennyMatching>;
    using BanditAlgorithm = Exp3p<Model>;
    using TreeBandit = TreeBandit<BanditAlgorithm>;

    // MoldState mold_state(3);
    PennyMatching game;
    prng device(0);
    Model model(device);

    MatrixNode<BanditAlgorithm> root;
    /*
    TreeBandit will derive to TreeBanditMultiThreaded(Pool) so we want to accept Algorithm as template param
    */



    TreeBandit session(device);
    session.run(
        1000000, game, model, root
    );

    math::print(root.stats.row_visits, 2);

    return 0;
}
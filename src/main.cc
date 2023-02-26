
#include <assert.h>
#include <iostream>

#include "state/test_states.hh"
#include "model/monte_carlo.hh"
#include "search/exp3p.hh"
// #include "tree/node.hh"
// #include "search/matrix_ucb.hh"

/*
Example of applying MatrixUCB search
*/

int main()
{
    using MoldState = MoldState<3>;
    using MonteCarlo = MonteCarlo<MoldState>;
    using Exp3p = Exp3p<MonteCarlo>;

    MoldState state(3);
    prng device(0);
    MonteCarlo model(device);
    auto state_ = state;
    state_.get_player_actions();
    model.inference(state_);

    Exp3p session(device);

    



}
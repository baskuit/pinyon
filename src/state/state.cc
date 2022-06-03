#pragma once

#include <string.h>
#include <iostream>

#include "../libsurskit/math.hh"
#include "state.hh"

PairActions :: ~PairActions () {
    delete [] actions0;
    delete [] actions1;
}


    // shallow copy will not work since we modify the strategies during transition
SolvedState :: SolvedState (SolvedState const& info) {
    terminal = info.terminal;
    rows = info.rows;
    cols = info.cols;
    payoff = info.payoff;
    strategy0 = new float[rows];
    strategy1 = new float[cols];
    memcpy(strategy0, info.strategy0, rows*sizeof(float)); 
    memcpy(strategy1, info.strategy1, cols*sizeof(float));
};

SolvedState :: ~SolvedState () {
    delete [] strategy0;
    delete [] strategy1;
}


    // State

SolvedState :: copy () {
    State* x = new State(device.copy());
    return x;
}

    virtual PairActions actions () {
        return PairActions();
    }
    virtual void actions (PairActions actions) {}
    virtual StateTransitionData transition(Action action0, Action action1) {return StateTransitionData();};
    virtual float rollout() {return 0.5f;};

};
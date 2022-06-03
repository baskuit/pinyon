#include "toy_state.hh"

ToyState* ToyState :: copy () {
    ToyState* x = new ToyState (id, pp, length, payoff);
    for (int i = 0; i < rows; ++i) {
        x->strategy0[i] = strategy0[i];
    }
    for (int j = 0; j < cols; ++j) {
        x->strategy1[j] = strategy1[j];
    }
    return x;
}

PairActions ToyState :: actions () {
    Action* two = new Action[2];
    two[0] = 0;
    two[1] = 1;
    return PairActions(2, 2, two, two);
}
#include "toy_state.hh"
#include <iostream>

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

StateTransitionData ToyState::transition(Action action0, Action action1) {
    StateTransitionData data;
    if (id == 'u') {
        if (pp == 0) {
            terminal = true;
            payoff = 0;
        } else {
            if (action0 == 0) {
                if (action1 == 0) {
                    pp -= 1;
                    payoff = pp/(float) (pp + 1);
                } else {
                    terminal = true;
                    payoff = 1;
                }
            } else {
                if (action1 == 0) {
                    terminal = true;
                    payoff = 1;
                } else {
                    terminal = true;
                    payoff = 0;                    
                }
            }
        }
    } else if (id == 's') {

    }

    return data;
}

float ToyState :: rollout () {
    PairActions pair = this->actions();
    while (!terminal) {
        int row_idx = this->device.random_int(pair.rows);
        int col_idx = this->device.random_int(pair.cols);
        this->transition(pair.actions0[row_idx], pair.actions1[col_idx]);
        //std::cout << row_idx << ' ' << col_idx << std::endl;
    }
    return this->payoff;
}
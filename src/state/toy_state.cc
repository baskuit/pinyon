#include "state.hh"
#include "toy_state.hh"

PairActions ToyState::actions () {
    Action two[2] = {0, 1};
    return PairActions(2, 2, two, two);
}

StateTransitionData ToyState::transition(Action action0, Action action1) {
    StateTransitionData data;
    if (this->info->id == 'u') {
        if (this->info->pp == 0) {
            this->info->terminal = true;
            this->info->payoff = 0;
        } else {
            if (action0 == 0) {
                if (action1 == 0) {
                    this->info->pp -= 1;
                    this->info->payoff = this->info->pp/(float) (this->info->pp + 1);
                } else {
                    this->info->terminal = true;
                    this->info->payoff = 1;
                }
            } else {
                if (action1 == 0) {
                    this->info->terminal = true;
                    this->info->payoff = 1;
                } else {
                    this->info->terminal = true;
                    this->info->payoff = 0;                    
                }
            }
        }
    } else if (this->info->id == 's') {

    }

    return data;
}

float ToyState::rollout () {
    while (this->info->terminal == false) {
        int row_idx = device.random_int(this->info->rows);
        int col_idx = device.random_int(this->info->cols);
        this->transition(row_idx, col_idx);
    }
    return this->info->payoff;
}

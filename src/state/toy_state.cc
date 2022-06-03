#include "state.hh"
#include "toy_state.hh"

ToyStateInfo :: ToyStateInfo (char id, int pp, int length, float payoff) :
    SolvedStateInfo(2, 2, payoff), id(id), pp(pp), length(length) {};

ToyStateInfo* ToyStateInfo :: copy () {
    ToyStateInfo* info = new ToyStateInfo();
    this->terminal = info->terminal;
    rows = info->rows;
    cols = info->cols;
    payoff = info->payoff;
    strategy0 = new float[rows];
    strategy1 = new float[cols];
    memcpy(strategy0, info->strategy0, rows*sizeof(float)); 
    memcpy(strategy1, info->strategy1, cols*sizeof(float));
    return info;
};

ToyState :: ToyState (ToyStateInfo* info) : 
    info(info) {}
    
ToyState :: ToyState (ToyStateInfo* info, prng device) : 
    State(info, device), info(info) {};

ToyState :: ToyState (char id, int pp, int length, float payoff) :
    info(new ToyStateInfo(id, pp, length, payoff)) {};

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

ToyState* ToyState :: copy () {
    ToyStateInfo* info_ = info->copy();
    ToyState* x = new ToyState(info_);
    return x;
}
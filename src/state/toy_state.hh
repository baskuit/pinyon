#include "state.hh"

#include <iostream>
// All this state stuff can stay off the heap

struct ToyStateInfo : SolvedStateInfo {

    char id;
    int pp;
    int length;

    ToyStateInfo(char id, int pp, int length) :
    SolvedStateInfo(2, 2, nullptr, nullptr, .5), id(id), pp(pp), length(length) {};

    ToyStateInfo (char id, int pp, int length, float payoff) :
    SolvedStateInfo(2, 2, nullptr, nullptr, payoff), id(id), pp(pp), length(length) {};

    ToyStateInfo (ToyStateInfo const& info) {
        terminal = info.terminal;
        rows = info.rows;
        cols = info.cols;
        payoff = info.payoff;
        strategy0 = new float[rows];
        strategy1 = new float[cols];
        memcpy(strategy0, info.strategy0, rows*sizeof(float)); 
        memcpy(strategy1, info.strategy1, cols*sizeof(float));
    };
};


class ToyState : public State {
public: 

    ToyStateInfo* info; 

    ToyState (ToyStateInfo* info) : 
    info(info) {}
    ToyState (ToyStateInfo* info, prng device) : 
    State(info, device), info(info) {};
    ToyState (char id, int pp, int length, float payoff) :
    info(*ToyStateInfo(id, pp, length, payoff)) {};

    PairActions actions ();

    StateTransitionData transition(Action action0, Action action1);

    float rollout ();

    ToyState* copy () {
        ToyState* x = new ToyState(info, device.copy());
        return x;
    }

};
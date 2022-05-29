#include "state.hh"

#include <iostream>
// All this state stuff can stay off the heap

struct ToyStateInfo : SolvedStateInfo {

    char id;
    int pp;
    int length;

    //ToyStateInfo ( ToyStateInfo const& T) {}; 

    ToyStateInfo(char id, int pp, int length) :
    id(id), pp(pp), length(length) {};

    ToyStateInfo () :
    id('u'), pp(2), length(0), SolvedStateInfo() {};

    ToyStateInfo (char id, int pp, int length, float payoff) :
    SolvedStateInfo(2, 2, nullptr, nullptr, payoff), id(id), pp(pp), length(length) {};
};


class ToyState : public State {
public: 
    ToyStateInfo info; 

    ToyState (ToyStateInfo info) {
        this->info = info;
        // TODO check that device is initialized properly 
    };
    ToyState(ToyStateInfo info, prng device) {
        this->info = info;
        this->device = device;
    };

    PairActions actions ();

    StateTransitionData transition(Action action0, Action action1);

    float rollout ();

};
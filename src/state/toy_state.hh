#include "state.hh"

#include <iostream>
// All this state stuff can stay off the heap

struct ToyStateInfo : SolvedStateInfo {

    char id;
    int pp;
    int length;

    // Why have a contstructor that will just cause cryptic Seg Faults?
    //ToyStateInfo(char id, int pp, int length) :
    //SolvedStateInfo(2, 2, nullptr, nullptr, .5), id(id), pp(pp), length(length) {};
    ToyStateInfo (char id, int pp, int length, float payoff);

    ToyStateInfo* copy ();
};


class ToyState : public State {
public: 

    ToyStateInfo* info; 

    ToyState (ToyStateInfo* info);
    ToyState (ToyStateInfo* info, prng device);
    ToyState (char id, int pp, int length, float payoff);

    PairActions actions ();

    StateTransitionData transition(Action action0, Action action1);

    float rollout ();

    ToyState* copy ();

};
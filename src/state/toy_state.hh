#include "state.hh"

#include <iostream>
// All this state stuff can stay off the heap

struct ToyStateInfo : SolvedStateInfo {

    char id;
    int pp;
    int length;

    ToyStateInfo (char id, int pp, int length, float payoff);
    ToyStateInfo () {};

    ToyStateInfo* copy ();
};


class ToyState : public State {
public: 

    ToyState (ToyStateInfo* info);
    ToyState (ToyStateInfo* info, prng device);
    ToyState (char id, int pp, int length, float payoff);

    PairActions actions ();

    StateTransitionData transition(Action action0, Action action1);

    float rollout ();

    ToyState* copy ();

};
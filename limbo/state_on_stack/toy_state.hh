#include "state.hh"

class ToyState : public SolvedState  {
public:
    char id;
    int pp;
    int length;

    ToyState (char id, int pp, int length, float payoff) :
    SolvedState(2, 2, payoff), id(id), pp(pp), length(length) {
    };

    ToyState* copy ();

    PairActions* actions ();
    StateTransitionData transition (Action actions0, Action actions1);
    float rollout ();
};
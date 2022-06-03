#include "state.hh"

class ToyState : SolvedState  {
public:
    char id;
    int pp;
    int length;

    ToyState (char id, int pp, int length, float payoff) :
    id(id), pp(pp), length(length) {
        this->rows = 2;
        this->cols = 2;
        this->terminal = false;
        this->payoff = payoff;
    };

    ToyState* copy ();

    PairActions actions ();
};
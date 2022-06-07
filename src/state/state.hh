#pragma once

#include <string.h>

#include "../libsurskit/math.hh"

typedef int Action;

typedef int Hash;

// Intended to be on stack
struct PairActions {

    int rows = 0;
    int cols = 0;
    Action* actions0 = nullptr;
    Action* actions1 = nullptr;

    PairActions () {};
    PairActions (int rows, int cols, Action* actions0, Action* actions1) :
        rows(rows), cols(cols), actions0(actions0), actions1(actions1) {};
};

struct StateTransitionData {

    Hash transitionKey = 0;
    Rational transitionProb;

    StateTransitionData () {}
    StateTransitionData (Hash transitionKey, Rational transitionProb) :
        transitionKey(transitionKey), transitionProb(transitionProb) {}
};

class State {
public:

    prng* device;
    float payoff = 0.5f; //garbage unless terminal

    State (prng* device) : device(device) {}
    State (prng* device, float payoff) : device(device), payoff(payoff) {}

    virtual PairActions* actions () = 0;
    virtual void actions (PairActions& actions) = 0;

    virtual StateTransitionData transition(Action action0, Action action1) = 0;

};

class SolvedState : public State {
public:

    bool terminal = true;
    int rows = 0;
    int cols = 0;
    float* strategy0 = nullptr;
    float* strategy1 = nullptr;

    SolvedState (prng* device) : State(device) {}
    SolvedState (prng* device, bool terminal, int rows, int cols, float payoff) :
        State(device, payoff), terminal(terminal), rows(rows), cols(cols) {}

    // virtual SolvedState* copy () ;

    // virtual float rollout () {return .5f;};
};
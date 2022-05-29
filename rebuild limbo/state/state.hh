#pragma once

#include "../libsurskit/math.hh"
#include "../libsurskit/rational.hh"
#include <vector>

#include <iostream>

typedef int Action;

struct StateTransitionData {
    int transitionKey;
    Rational transitionProb;
};

struct PairActions {
    int rows;
    int cols;
    Action* actions0;
    Action* actions1;
    PairActions () {};
    PairActions (int rows, int cols, Action* actions0, Action* actions1) :
    rows(rows), cols(cols), actions0(actions0), actions1(actions1) {};
};
// is this correct? for deleting?

class State {
public:

    // analytically
    bool terminal;
    float payoff;
    float* nash0;
    float* nash1;
    ~State(){
        delete nash0;
        delete nash1;
    };
    State() {};
    State(State& s) :
    terminal(s.terminal), payoff(s.payoff), nash0(s.nash0), nash1(s.nash1) {};

    virtual State* copy () {
        return new State();
    }

    virtual PairActions actions () {
        return PairActions();
    }
    virtual void actions (PairActions) {}

    virtual StateTransitionData transition(Action action0, Action action1) {return StateTransitionData();};
    virtual StateTransitionData transition(PairActions pair) {return StateTransitionData();};
    virtual float rollout() {return 0.5f;};

};
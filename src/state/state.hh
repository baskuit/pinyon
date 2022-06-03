#pragma once

#include <string.h>
#include <iostream>

#include "../libsurskit/math.hh"


    // Data


typedef int Action;

typedef int Hash;

struct PairActions {
    int rows;
    int cols;
    Action* actions0;
    Action* actions1;

    PairActions () :
    rows(0), cols(0), actions0(nullptr), actions1(nullptr) {};
    PairActions (int rows, int cols, Action* actions0, Action* actions1) :
    rows(rows), cols(cols), actions0(actions0), actions1(actions1) {};
};

struct StateTransitionData {
    Hash transitionKey;
    Rational transitionProb;
    StateTransitionData () : transitionKey(0) {}; //check if tP is properly initialized to 1/1
    StateTransitionData (Hash transitionKey, Rational transitionProb) :
    transitionKey(transitionKey), transitionProb(transitionProb) {};
};


    // State


class State {
public:

    prng device; //default copy keeps the progress too, copy() does not
    // thus state default copy is deep copy

    State () {};
    State (prng device) : device(device) {};

    virtual State* copy () {return this;};

    virtual PairActions actions () {
        return PairActions();
    }
    virtual void actions (PairActions actions) {}
    virtual StateTransitionData transition(Action action0, Action action1) {return StateTransitionData();};
    virtual float rollout() {return 0.5f;};

};

class SolvedState : public State {
public:
    bool terminal;
    int rows;
    int cols;
    float* strategy0;
    float* strategy1;
    float payoff;
    
    // iirc, If I remove the empty constructor, I get errors?
    SolvedState () {

    };
    SolvedState (int rows, int cols, float payoff) :
    terminal(rows * cols == 0), rows(rows), cols(cols), strategy0(new float[rows]), strategy1(new float[cols]), payoff(payoff) {};
    SolvedState (int rows, int cols, float* strategy0, float* strategy1, float payoff) :
    terminal(rows * cols == 0), rows(rows), cols(cols), strategy0(strategy0), strategy1(strategy1), payoff(payoff) {};

    ~SolvedState ();

    virtual SolvedState* copy () {return this;};

    virtual float rollout () {return .5f;};
};
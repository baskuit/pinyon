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
    ~PairActions () {
        delete [] actions0;
        delete [] actions1;
    }
};

struct StateTransitionData {
    Hash transitionKey;
    Rational transitionProb;
    StateTransitionData () : transitionKey(0) {}; //check if tP is properly initialized to 1/1
    StateTransitionData (Hash transitionKey, Rational transitionProb) :
    transitionKey(transitionKey), transitionProb(transitionProb) {};
};


    // StateInfo


struct StateInfo {};

struct SolvedStateInfo : StateInfo {

    bool terminal;
    int rows;
    int cols;
    float* strategy0;
    float* strategy1;
    float payoff;

    SolvedStateInfo () :
    terminal(true), rows(0), cols(0), strategy0(nullptr), strategy1(nullptr), payoff(.5) {};
    SolvedStateInfo (bool terminal, int rows, int cols, float* strategy0, float* strategy1, float payoff) :
    terminal(terminal), rows(rows), cols(cols), strategy0(strategy0), strategy1(strategy1), payoff(payoff) {
        terminal = (rows * cols == 0);
    };
    SolvedStateInfo (int rows, int cols, float* strategy0, float* strategy1, float payoff) :
    terminal(rows * cols == 0), rows(rows), cols(cols), strategy0(strategy0), strategy1(strategy1), payoff(payoff) {};

    SolvedStateInfo (SolvedStateInfo const& info) {
        terminal = info.terminal;
        rows = info.rows;
        cols = info.cols;
        payoff = info.payoff;
        strategy0 = new float[rows];
        strategy1 = new float[cols];
        memcpy(strategy0, info.strategy0, rows*sizeof(float)); 
        memcpy(strategy1, info.strategy1, cols*sizeof(float));
    };

    ~SolvedStateInfo () {
        delete [] strategy0;
        delete [] strategy1;
    }
};


    // State


class State {
public:

    StateInfo info;
    prng device;

    State () {};
    State(StateInfo info, prng device) : info(info), device(device) {};
    State(StateInfo info) : info(info) {}; // TODO does prng device; initilialize?
    //State (State const& state) {}; 

    virtual PairActions actions () {
        return PairActions();
    }
    virtual void actions (PairActions actions) {}
    virtual StateTransitionData transition(Action action0, Action action1) {return StateTransitionData();};
    virtual float rollout() {return 0.5f;};

};
#pragma once

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
    PairActions (const PairActions &pair) {
        rows = pair.rows;
        cols = pair.cols; 
    }   
};

struct StateTransitionData {
    Hash transitionKey;
    Rational transitionProb;
    StateTransitionData () : transitionKey(0) {}; //check if tP is properly initialized to 1/1
    StateTransitionData (int transitionKey, Rational transitionProb) :
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
    terminal(true), rows(rows), cols(cols), strategy0(nullptr), strategy1(nullptr), payoff(.5) {};
    SolvedStateInfo (bool terminal, int rows, int cols, float* strategy0, float* strategy1, float payoff) :
    terminal(terminal), rows(rows), cols(cols), strategy0(strategy0), strategy1(strategy1), payoff(payoff) {
        terminal = (rows * cols == 0);
    };
    SolvedStateInfo (int rows, int cols, float* strategy0, float* strategy1, float payoff) :
    rows(rows), cols(cols), strategy0(strategy0), strategy1(strategy1), payoff(payoff) {
        terminal = (rows * cols == 0);
    };
};


    // State


class State {
public:

    StateInfo info;
    prng device;

    State () {};
    State(StateInfo info, prng device) : info(info), device(device) {};
    State(StateInfo info) : info(info) {}; // TODO does prng device; initilialize?
    ~State () {};
    //State ( State const& T) {}; 

    virtual PairActions actions () {
        return PairActions();
    }
    virtual void actions (PairActions actions) {}
    virtual StateTransitionData transition(Action action0, Action action1) {return StateTransitionData();};
    virtual float rollout() {return 0.5f;};

};
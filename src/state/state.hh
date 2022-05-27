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


// is this correct? for deleting?
struct Actions {
    Action* actions0;
    Action* actions1;
    Actions () {}
    ~Actions() {
        delete actions0;
        delete actions1;
    }
};

class State {
public:
    int rows;
    int cols;

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
    rows(s.rows), cols(s.cols), terminal(s.terminal), payoff(s.payoff), nash0(s.nash0), nash1(s.nash1) {};

    virtual State* copy () {
        return new State();
    };
    // this->terminal is instead achieved by first 
    // filling actions vectors and checking if still empty
    //virtual Actions actions(std::vector<Action>* row_actions, std::vector<Action>* col_actions) {return Actions()};
    virtual StateTransitionData transition(Action action0, Action action1) {return StateTransitionData();};

    // play random moves until terminal and return 'value0' payoff
    virtual float rollout() {std::cout << "base rollout" << std::endl; return .5f;};
    // this canonical function only returns value0 since value1 = 1 - value0

};
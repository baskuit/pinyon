#pragma once

#include "../libsurskit/math.hh"
#include "../libsurskit/rational.hh"
#include <vector>

typedef int Action;

struct StateTransitionData {
    int transitionKey;
    Rational transitionProb;
};

class State {
public:
    int rows;
    int cols;

    State();
    State(const State&);

    // this->terminal is instead achieved by first 
    // filling actions vectors and checking if still empty
    void actions(std::vector<Action>* row_actions, std::vector<Action>* col_actions);
    StateTransitionData transition(Action action0, Action action1);

    // play random moves until terminal and return 'value0' payoff
    float rollout();
    // this canonical function only returns value0 since value1 = 1 - value0

};
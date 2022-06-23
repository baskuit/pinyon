#pragma once

#include <string.h>

#include "../libsurskit/math.hh"


// PairActions


template <int size, typename Action>
struct PairActions {
    int rows = 0;
    int cols = 0;
    std::array<Action, size> actions0;
    std::array<Action, size> actions1;
    PairActions () {} 
    PairActions (int rows, int cols) :
        rows(rows), cols(cols) {}
};


// TransitionData


template <typename Hash>
struct TransitionData {

    Hash key;
    Rational probability;

    TransitionData () {} // Dangerous? Hash type must have default constructor, I believe.
    TransitionData (Hash key, Rational probability) :
        key(key), probability(probability) {}
};


// State


template <int size, typename Action, typename Hash>
class State {
public:

    static const int size_ = size;
    using action_t = Action;
    using hash_t = Hash;
    using pair_actions_t = PairActions<size, Action>;
    using transition_data_t = TransitionData<Hash>;

    prng& device;
    double payoff0 = 0.5f;
    double payoff1 = 0.5f;

    State (prng& device) : 
        device(device) {}
    State (prng& device, double payoff) : 
        device(device), payoff0(payoff), payoff1(1-payoff) {}
    State (prng& device, double payoff0, double payoff1) : 
        device(device), payoff0(payoff0), payoff1(payoff1) {}

    virtual pair_actions_t actions () = 0;
    virtual void actions (pair_actions_t& pair) = 0;
    virtual transition_data_t transition(Action action0, Action action1) = 0;
};



// // Solved State


template <int size, typename Action, typename Hash>
class SolvedState : public State<size, Action, Hash> {
public:

    bool terminal = true;
    int rows = 0;
    int cols = 0;
    std::array<double, size> strategy0;
    std::array<double, size> strategy1;

    SolvedState (prng& device, double payoff, int rows, int cols) :
        State<size, Action, Hash>(device, payoff), terminal(rows*cols==0), rows(rows), cols(cols) {}
    SolvedState (prng& device, double payoff0, double payoff1, int rows, int cols) :
        State<size, Action, Hash>(device, payoff0, payoff1), terminal(rows*cols==0), rows(rows), cols(cols) {}
};
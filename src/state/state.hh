#pragma once

#include <string.h>

#include "../libsurskit/math.hh"


// PairActions


template <int size, typename Action>
struct PairActions {

    /*
    Legal actions for players 0 and 1.
    Stored in each matrix node, for example
    Actions a_i player 0 after i=rows are garbage, ditto for cols, player 1.
    */

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

    /*
    After applying a pair of actions to a matrix node, we stochastically transition to another matrix node
    and recieve some identifying observation of type Hash.
    Hash is essentially the type of the chance player's actions.
    We use rational numbers to quantify the probability of that transition
    This has nice properties; for example, the sum of probabilities at a chance node 
    equals 1 if and only if all actions of the chace player have been explored.
    */

    TransitionData () {}
    TransitionData (Hash key, Rational probability) :
        key(key), probability(probability) {}
};


// State


template <int size, typename Action, typename Hash>
class State {
public:

    /*
    Markov Desicion Process.
    We assume rewards at each transition are 0 until the end until the last
    Rewards at the end are called payoffs. In the example code and motivating papers 
    we assume payoff0 + payoff1 = 1. 
    */

    static const int size_ = size;
    using action_t = Action;
    using hash_t = Hash;
    using pair_actions_t = PairActions<size, Action>;
    using transition_data_t = TransitionData<Hash>;

    prng& device;
    double payoff0 = 0.5f;
    double payoff1 = 0.5f;
    // Currently reward type is 'hard'-coded. Float is not accurate enough, and no need for e.g. Rational type.

    State (prng& device) : 
        device(device) {}
    State (prng& device, double payoff) : 
        device(device), payoff0(payoff), payoff1(1-payoff) {}
    State (prng& device, double payoff0, double payoff1) : 
        device(device), payoff0(payoff0), payoff1(payoff1) {}
    // The overarching search function is given a state that is then copied for each rollout.
    // We use copy constructors for this

    virtual pair_actions_t get_legal_actions () = 0;
    virtual void get_legal_actions (pair_actions_t& legal_actions) = 0;
    virtual transition_data_t apply_actions(Action action0, Action action1) = 0;
};


// Solved State


template <int size, typename Action, typename Hash>
class SolvedState : public State<size, Action, Hash> {
public:

    /*
    State where a given Nash equilibrium is known.
    We then also know the legal actions and terminality too
    so we store this info as members.
    */

    bool is_terminal = true;
    int rows = 0;
    int cols = 0;
    std::array<double, size> strategy0;
    std::array<double, size> strategy1;

    SolvedState (prng& device, double payoff, int rows, int cols) :
        State<size, Action, Hash>(device, payoff), is_terminal(rows*cols==0), rows(rows), cols(cols) {}
    SolvedState (prng& device, double payoff0, double payoff1, int rows, int cols) :
        State<size, Action, Hash>(device, payoff0, payoff1), is_terminal(rows*cols==0), rows(rows), cols(cols) {}
};
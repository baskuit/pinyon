#pragma once

#include <string.h>

#include "../libsurskit/math.hh"


typedef int Hash;

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

template <typename Hash>
struct StateTransitionData {

    Hash transitionKey = 0;
    Rational transitionProb;

    StateTransitionData () {}
    StateTransitionData (Hash transitionKey, Rational transitionProb) :
        transitionKey(transitionKey), transitionProb(transitionProb) {}
};


// State


template <int size, typename Action, typename Hash>
class State {
public:

    static const int array_size = size;
    typedef Action action_type;
    typedef PairActions<size, Action> pair_actions_type;

    prng& device;
    double payoff0 = 0.5f; //garbage unless terminal
    double payoff1 = 0.5f;

    State<size, Action> (prng& device) : 
        device(device) {}
    State<size, Action> (prng& device, double payoff) : 
        device(device), payoff0(payoff), payoff1(1-payoff) {}

    virtual PairActions<size, Action> actions () = 0;
    virtual StateTransitionData transition(Action action0, Action action1) = 0;

};


// Solved State


template <int size, typename Action>
class SolvedState : public State<size, Action> {
public:

    bool terminal = true;
    int rows = 0;
    int cols = 0;
    std::array<double, size> strategy0;
    std::array<double, size> strategy1;

    SolvedState (prng& device, double payoff, int rows, int cols) :
        State<size, Action>(device, payoff), terminal(rows*cols==0), rows(rows), cols(cols) {}

};


// Toy State


template <int size>
class ToyState : public SolvedState<size, int> {
public:

    char id = 'u';
    int pp = 1;
    int length = 0;

    ToyState (prng& device) :
        SolvedState<size, int>(device, .5f, 2, 2) {}
    ToyState (prng& device, char id, int pp, int length) :
        SolvedState<size, int>(device, .5f, 2, 2), id(id), pp(pp), length(length) {}

    PairActions<size, int> actions () { // Doesn't this have to be derived here, otherwise you cant use the answer
        PairActions<size, int> pair;
        if (this->terminal) {
            pair.rows = 0;
            pair.cols = 0;
            return pair;
        }
        pair.rows = 2;
        pair.cols = 2;
        for (int i = 0; i < 2; ++i) {
            pair.actions0[i] = i;
            pair.actions1[i] = i;
        };
        return pair;
    }

    StateTransitionData transition (int action0, int action1) {
        StateTransitionData x;

        if (id == 'u') {

                if (pp == 0) {
                    this->payoff0 = 0;
                    this->terminal = true;
                    return x;
                }

                if (action0 == 0) {
                    --pp;
                    if (action1 == 0) {
                    } else {
                        this->payoff0 = 1;
                        this->terminal = true; 
                    }
                } else {
                    if (action1 == 0) {
                        this->payoff0 = 1;
                        this->terminal = true;
                    } else {
                        this->payoff0 = 0;
                        this->terminal = true;
                    }
                }

            this->strategy0[0] = pp/(double)(pp+1);
            this->strategy0[1] = 1/(double)(pp+1);
            this->strategy1[0] = pp/(double)(pp+1);
            this->strategy1[1] = 1/(double)(pp+1);

        } else if (id == 's') {

                if (action0 == 0) {
                    if (length == 0) {
                        this->payoff0 = 1.f;
                        this->terminal = true;
                    }
                    --length;
                } else {
                    this->payoff0 = 0.f;
                    this->terminal = true;
                }

            this->strategy0[0] = 1;
            this->strategy0[1] = 0;

        } else if (id == 't') {

                if (action1 == 0) {
                    if (length == 0) {
                        this->payoff0 = 0.f;
                        this->terminal = true;
                    }
                    --length;
                } else {
                    this->payoff0 = 1.f;
                    this->terminal = true;
                }

            this->strategy1[0] = 1;
            this->strategy1[1] = 0;

        } else if (id == '2') {

            x.transitionKey = this->device.random_int(2);
            x.transitionProb = {1, 2};

        } else if (id == 'w') {

                if (pp == 0) {
                    this->payoff0 = 0;
                    this->terminal = true;
                    return x;
                }

                if (action0 == 0) {
                    --pp;
                    if (action1 == 0) {
                        
                    } else {
                        this->payoff0 = 1;
                        this->id = 's';
                    }
                } else {
                    if (action1 == 0) {
                        this->payoff0 = 1;
                        this->id = 's';
                    } else {
                        this->payoff0= 0;
                        this->id = 't';
                    }
                }

            this->strategy0[0] = pp/(double)(pp+1);
            this->strategy0[1] = 1/(double)(pp+1);
            this->strategy1[0] = pp/(double)(pp+1);
            this->strategy1[1] = 1/(double)(pp+1);

        }

        return x;
    }

};
 
template <int size>
class MoldState : public State<size, int> {
public:

    int depth = 0;

    MoldState<size> (prng& device, int depth) :
        State<size, int>(device), depth(depth) {}

    PairActions<size, int> actions () {
        PairActions<size, int> pair;
        if (depth == 0) {
            pair.rows = 0;
            pair.cols = 0;
            return pair;
        }
        pair.rows = 2;
        pair.cols = 2;
        for (int i = 0; i < 2; ++i) {
            pair.actions0[i] = i;
            pair.actions1[i] = i;
        };
        return pair;
    }

    StateTransitionData transition (int action0, int action1) {
        --depth;
        StateTransitionData x;
        return x;
    }

};
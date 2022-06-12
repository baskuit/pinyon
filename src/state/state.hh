#pragma once

#include <string.h>

#include "../libsurskit/math.hh"

typedef int Action;

typedef int Hash;

template <int size>
struct PairActions {

    int rows = 0;
    int cols = 0;
    std::array<Action, size> actions0;
    std::array<Action, size> actions1;

    PairActions () {};
    PairActions (int rows, int cols) :
        rows(rows), cols(cols) {};
};

struct StateTransitionData { // does this copy (return by value) correctly?

    Hash transitionKey = 0;
    Rational transitionProb;

    StateTransitionData () {}
    StateTransitionData (Hash transitionKey, Rational transitionProb) :
        transitionKey(transitionKey), transitionProb(transitionProb) {}
};


// State


template <int size>
class State {
public:

    prng& device;
    double payoff = 0.5f; //garbage unless terminal

    State (prng& device) : 
        device(device) {}
    State (prng& device, double payoff) : 
        device(device), payoff(payoff) {}

    virtual double rollout () = 0;
    virtual void actions (PairActions<size>& actions) = 0;
    virtual StateTransitionData transition(Action action0, Action action1) = 0;

};


// Solved State


template <int size>
class SolvedState : public State<size> {
public:

    bool terminal = true;
    int rows = 0;
    int cols = 0;
    std::array<double, size> strategy0;
    std::array<double, size> strategy1;

    SolvedState<size> (prng& device, double payoff, int rows, int cols) :
        State<size>(device, payoff), terminal(rows*cols==0), rows(rows), cols(cols) {}
    //virtual void actions (PairActions<size>& actions) = 0;
};


// Toy State


template <int size>
class ToyState : public SolvedState<size> {
public:

    char id = 'u';
    int pp = 1;
    int length = 0;

    ToyState<size> (prng& device) :
        SolvedState<size>(device, .5f, 2, 2) {}
    ToyState<size> (prng& device, char id, int pp, int length) :
        SolvedState<size>(device, .5f, 2, 2), id(id), pp(pp), length(length) {}

    void actions (PairActions<size>& pair) {
        if (this->terminal) {
            pair.rows = 0;
            pair.cols = 0;
            return;
        }
        pair.rows = 2;
        pair.cols = 2;
        for (int i = 0; i < 2; ++i) {
            pair.actions0[i] = i;
            pair.actions1[i] = i;
        };
    }

    StateTransitionData transition (Action action0, Action action1) {
        StateTransitionData x;

        if (id == 'u') {

                if (pp == 0) {
                    this->payoff = 0;
                    this->terminal = true;
                    return x;
                }

                if (action0 == 0) {
                    --pp;
                    if (action1 == 0) {
                    } else {
                        this->payoff = 1;
                        this->terminal = true; 
                    }
                } else {
                    if (action1 == 0) {
                        this->payoff = 1;
                        this->terminal = true;
                    } else {
                        this->payoff = 0;
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
                        this->payoff = 1.f;
                        this->terminal = true;
                    }
                    --length;
                } else {
                    this->payoff = 0.f;
                    this->terminal = true;
                }

            this->strategy0[0] = 1;
            this->strategy0[1] = 0;

        } else if (id == 't') {

                if (action1 == 0) {
                    if (length == 0) {
                        this->payoff = 0.f;
                        this->terminal = true;
                    }
                    --length;
                } else {
                    this->payoff = 1.f;
                    this->terminal = true;
                }

            this->strategy1[0] = 1;
            this->strategy1[1] = 0;

        } else if (id == '2') {

            x.transitionKey = this->device.random_int(2);
            x.transitionProb = {1, 2};

        } else if (id == 'w') {

                if (pp == 0) {
                    this->payoff = 0;
                    this->terminal = true;
                    return x;
                }

                if (action0 == 0) {
                    --pp;
                    if (action1 == 0) {
                        
                    } else {
                        this->payoff = 1;
                        this->id = 's';
                    }
                } else {
                    if (action1 == 0) {
                        this->payoff = 1;
                        this->id = 's';
                    } else {
                        this->payoff = 0;
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

    double rollout () {
        PairActions<size> pair;
        while (!this->terminal) {
            Action action0 = this->device.random_int(2);
            Action action1 = this->device.random_int(2);
            this->transition(action0, action1);
        }
        return this->payoff;
    }

};

template <int size>
class MoldState : public State<size> {
public:

    int depth = 0;

    MoldState<size> (prng& device, int depth) :
        State<size>(device), depth(depth) {}

    void actions (PairActions<size>& pair) {
        if (depth == 0) {
            pair.rows = 0;
            pair.cols = 0;
            return;
        }
        pair.rows = 2;
        pair.cols = 2;
        for (int i = 0; i < 2; ++i) {
            pair.actions0[i] = i;
            pair.actions1[i] = i;
        };
    }

    StateTransitionData transition (Action action0, Action action1) {
        --depth;
        StateTransitionData x;
        return x;
    }

    double rollout () {
        PairActions<size> pair;
        while (depth > 0) {
            Action action0 = this->device.random_int(2);
            Action action1 = this->device.random_int(2);
            this->transition(action0, action1);
        }
        return this->payoff;
    }

};
#pragma once

#include "state.hh"


// ToyState. A contrived game where the payoff and strategies are always known. Used to test search algorithms.


template <int size>
class ToyState : public SolvedState<size, int, int> {
public:

    char id = 'u';
    int pp = 1;
    int length = 0;

    ToyState (prng& device) :
        SolvedState<size, int, int>(device, .5f, 2, 2) {} // TODO: Need to calculate payoff, strategies on initialization too
    ToyState (prng& device, char id, int pp, int length) :
        SolvedState<size, int, int>(device, .5f, 2, 2), id(id), pp(pp), length(length) {}

    typename ToyState::pair_actions_t actions () {
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

    void actions (typename ToyState::pair_actions_t& pair) {
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

    typename ToyState::transition_data_t transition (int action0, int action1) {
        typename ToyState::transition_data_t x(0, Rational());

        if (id == 'u') {

                if (pp == 0) {
                    this->payoff0 = 0;
                    this->payoff1 = 1;
                    this->terminal = true;
                    return x;
                }

                if (action0 == 0) {
                    --pp;
                    if (action1 == 0) {
                    } else {
                        this->payoff0 = 1;
                        this->payoff1 = 0;
                        this->terminal = true; 
                    }
                } else {
                    if (action1 == 0) {
                        this->payoff0 = 1;
                        this->payoff1 = 0;
                        this->terminal = true;
                    } else {
                        this->payoff0 = 0;
                        this->payoff1 = 1;
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

            x.key = this->device.random_int(2);
            x.probability = {1, 2};

        } else if (id == 'w') {

                if (pp == 0) {
                    this->payoff0 = 0;
                    this->payoff1 = 1;
                    this->terminal = true;
                    return x;
                }

                if (action0 == 0) {
                    --pp;
                    if (action1 == 0) {
                        
                    } else {
                        this->payoff0 = 1;
                        this->payoff1 = 0;
                        this->id = 's';
                    }
                } else {
                    if (action1 == 0) {
                        this->payoff0 = 1;
                        this->payoff1 = 0;
                        this->id = 's';
                    } else {
                        this->payoff0 = 0;
                        this->payoff1 = 1;
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
 

// Large tree for memory tests etc.


template <int size>
class MoldState : public State<size, int, int> {
public:

    int depth = 0;

    MoldState<size> (prng& device, int depth) :
        State<size, int, int>(device), depth(depth) {}

    typename MoldState::pair_actions_t actions () {
        typename MoldState::pair_actions_t pair;
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

    void actions (typename MoldState::pair_actions_t pair) {
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

    typename MoldState::transition_data_t transition (int action0, int action1) {
        --depth;
        typename MoldState::transition_data_t x;
        x.key = 0;
        return x;
    }

};
#pragma once

#include "state.hh"


// ToyState is a contrived game with known payoff and equilibria. 


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

    typename ToyState::pair_actions_t get_legal_actions () {
        PairActions<size, int> pair;
        if (this->is_terminal) {
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

    void get_legal_actions (typename ToyState::pair_actions_t& pair) {
        if (this->is_terminal) {
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

    typename ToyState::transition_data_t apply_actions (int action0, int action1) {
        typename ToyState::transition_data_t transition_data(0, Rational());

        if (id == 'u') {

            // "Sucker Punch" game

                if (pp == 0) {
                    this->payoff0 = 0;
                    this->payoff1 = 1;
                    this->is_terminal = true;
                    return transition_data;
                }

                if (action0 == 0) {
                    --pp;
                    if (action1 == 0) {
                    } else {
                        this->payoff0 = 1;
                        this->payoff1 = 0;
                        this->is_terminal = true; 
                    }
                } else {
                    if (action1 == 0) {
                        this->payoff0 = 1;
                        this->payoff1 = 0;
                        this->is_terminal = true;
                    } else {
                        this->payoff0 = 0;
                        this->payoff1 = 1;
                        this->is_terminal = true;
                    }
                }

            this->strategy0[0] = pp/(double)(pp+1);
            this->strategy0[1] = 1/(double)(pp+1);
            this->strategy1[0] = pp/(double)(pp+1);
            this->strategy1[1] = 1/(double)(pp+1);

        } else if (id == 's') {

            // Player 0 to win by player action=0

                if (action0 == 0) {
                    if (length == 0) {
                        this->payoff0 = 1;
                        this->payoff1 = 0;
                        this->is_terminal = true;
                    }
                    --length;
                } else {
                    this->payoff0 = 0;
                    this->payoff1 = 1;
                    this->is_terminal = true;
                }

            this->strategy0[0] = 1;
            this->strategy0[1] = 0;

        } else if (id == 't') {

            // Player 1 to win by playing action=0

                if (action1 == 0) {
                    if (length == 0) {
                    this->payoff0 = 0;
                    this->payoff1 = 1;
                        this->is_terminal = true;
                    }
                    --length;
                } else {
                    this->payoff0 = 1;
                    this->payoff1 = 0;
                    this->is_terminal = true;
                }

            this->strategy1[0] = 1;
            this->strategy1[1] = 0;

        } else if (id == '2') {

            transition_data.key = this->device.random_int(2);
            transition_data.probability = {1, 2};

        } else if (id == 'w') {

            // Sucker punch game where game end by K.O is replaced by corresponding 's' or 't' game.

                if (pp == 0) {
                    this->payoff0 = 0;
                    this->payoff1 = 1;
                    this->is_terminal = true;
                    return transition_data;
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

        return transition_data;
    }

};
 

// Large uniform tree for testing etc. So called because it spreads out until it can't.


template <int size>
class MoldState : public State<size, int, int> {
public:

    int depth = 0;

    MoldState<size> (prng& device, int depth) :
        State<size, int, int>(device), depth(depth) {}

    typename MoldState::pair_actions_t get_legal_actions () {
        typename MoldState::pair_actions_t pair;
        if (depth == 0) {
            pair.rows = 0;
            pair.cols = 0;
            return pair;
        }
        pair.rows = size;
        pair.cols = size;
        for (int i = 0; i < size; ++i) {
            pair.actions0[i] = i;
            pair.actions1[i] = i;
        };
        return pair;
    }

    void get_legal_actions (typename MoldState::pair_actions_t& pair) {
        if (depth == 0) {
            pair.rows = 0;
            pair.cols = 0;
            return;
        }
        pair.rows = size;
        pair.cols = size;
        for (int i = 0; i < size; ++i) {
            pair.actions0[i] = i;
            pair.actions1[i] = i;
        };
    }

    typename MoldState::transition_data_t apply_actions (int action0, int action1) {
        --depth;
        typename MoldState::transition_data_t x;
        x.key = 0;
        return x;
    }

};
#pragma once

#include "state.hh"


// ToyState is a contrived game with known payoff and equilibria. 

enum ToyStatePhase {
        sucker_punch,
        p0_win_by_0,
        p1_win_by_0,
        sucker_punch_win_by,
};

template <int size>
class ToyState : public SolvedState<size, int, int> {
public:

    ToyStatePhase phase = sucker_punch;
    int pp = 1;
    int length = 0;

    ToyState (prng& device) :
        SolvedState<size, int, int>(device, .5f, 2, 2) {} // TODO: Need to calculate payoff, strategies on initialization too
    ToyState (prng& device, ToyStatePhase phase, int pp, int length) :
        SolvedState<size, int, int>(device, .5f, 2, 2), phase(phase), pp(pp), length(length) {}

    typename ToyState::pair_actions_t get_legal_actions () {
        PairActions<size, int> legal_actions;
        if (this->is_terminal) {
            legal_actions.rows = 0;
            legal_actions.cols = 0;
            return legal_actions;
        }
        legal_actions.rows = 2;
        legal_actions.cols = 2;
        for (int i = 0; i < 2; ++i) {
            legal_actions.actions0[i] = i;
            legal_actions.actions1[i] = i;
        };
        return legal_actions;
    }

    void get_legal_actions (typename ToyState::pair_actions_t& legal_actions) {
        if (this->is_terminal) {
            legal_actions.rows = 0;
            legal_actions.cols = 0;
            return;
        }
        legal_actions.rows = 2;
        legal_actions.cols = 2;
        for (int i = 0; i < 2; ++i) {
            legal_actions.actions0[i] = i;
            legal_actions.actions1[i] = i;
        };
    }

    typename ToyState::transition_data_t apply_actions (int action0, int action1) {
        typename ToyState::transition_data_t transition_data(0, Rational());

        if (phase == sucker_punch) {

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

        } else if (phase == p0_win_by_0) {

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

        } else if (phase == p1_win_by_0) {

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

        } else if (phase == sucker_punch_win_by) {

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
                        this->phase = p0_win_by_0;
                    }
                } else {
                    if (action1 == 0) {
                        this->payoff0 = 1;
                        this->payoff1 = 0;
                        this->phase = p0_win_by_0;
                    } else {
                        this->payoff0 = 0;
                        this->payoff1 = 1;
                        this->phase = p1_win_by_0;
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
        typename MoldState::pair_actions_t legal_actions;
        if (depth == 0) {
            legal_actions.rows = 0;
            legal_actions.cols = 0;
            return legal_actions;
        }
        legal_actions.rows = size;
        legal_actions.cols = size;
        for (int i = 0; i < size; ++i) {
            legal_actions.actions0[i] = i;
            legal_actions.actions1[i] = i;
        };
        return legal_actions;
    }

    void get_legal_actions (typename MoldState::pair_actions_t& legal_actions) {
        if (depth == 0) {
            legal_actions.rows = 0;
            legal_actions.cols = 0;
            return;
        }
        legal_actions.rows = size;
        legal_actions.cols = size;
        for (int i = 0; i < size; ++i) {
            legal_actions.actions0[i] = i;
            legal_actions.actions1[i] = i;
        };
    }

    typename MoldState::transition_data_t apply_actions (int action0, int action1) {
        --depth;
        typename MoldState::transition_data_t x;
        x.key = 0;
        return x;
    }

};
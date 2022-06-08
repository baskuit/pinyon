#pragma once

#include "state.hh"

class ToyState : public SolvedState  {
public:
    char id = 'u';
    int pp = 1;
    int length = 0;

    ToyState (prng* device) :
        SolvedState(device, false, 2, 2, 0.5f) {}
    ToyState (prng* device, char id, int pp, int length, float payoff) :
        SolvedState(device, false, 2, 2, payoff), id(id), pp(pp), length(length) {}

    void copy (ToyState* state) {
        assert(state->rows == rows && state->cols == cols);
        state->id = id;
        state->pp = pp;
        state->length = length;

        state->terminal = terminal;
        state->rows = rows;
        state->cols = cols;
        state->payoff = payoff;
        // for (int row_idx = 0; row_idx < rows; ++row_idx) {
        //     state->strategy0[row_idx] = strategy0[row_idx];

        // }
        // for (int col_idx = 0; col_idx < cols; ++col_idx) {
        //     state->strategy1[col_idx] = strategy1[col_idx];
        // }
    };

    // returns nullptr
    PairActions* actions () {return nullptr;}
    
    void actions (PairActions& pair) {
        if (terminal) {
            pair.rows = 0;
            pair.cols = 0;
            /// more
        } else {
            pair.rows = 2;
            pair.cols = 2;
            pair.actions0[0] = 0;
            pair.actions0[1] = 1;
            pair.actions1[0] = 0;
            pair.actions1[1] = 1;
        }

    };

    StateTransitionData transition (Action action0, Action action1) {
        StateTransitionData data;
        if (id == 'u') {
            if (pp == 0) {
                terminal = true;
                payoff = 0;
            } else {
                if (action0 == 0) {
                    if (action1 == 0) {
                        pp -= 1;
                        payoff = pp/(float) (pp + 1);
                    } else {
                        terminal = true;
                        payoff = 1;
                    }
                } else {
                    if (action1 == 0) {
                        terminal = true;
                        payoff = 1;
                    } else {
                        terminal = true;
                        payoff = 0;                    
                    }
                }
            }
        } else if (id == 's') {

        }

        return data;
    };

};
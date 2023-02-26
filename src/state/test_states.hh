#pragma once

#include "state.hh"

// Large uniform tree for testing etc. So called because it spreads out until it can't.

template <int size>
class MoldState : public StateArray<size, int, int, double>
{
public:
    int depth;

    MoldState (int depth) : depth((depth >= 0) * depth) {}
    // MoldState (MoldState &t) {}

    void get_player_actions () {
        if (this->depth > 0) {
            for (int i = 0; i < size; ++i) {
                MoldState::pair_actions.row_actions[i] = i;
                MoldState::pair_actions.col_actions[i] = i;
            }
            MoldState::pair_actions.rows = size;
            MoldState::pair_actions.cols = size;
            this->is_terminal = false;
        } else {
            MoldState::pair_actions.rows = 0;
            MoldState::pair_actions.cols = 0;
            this->is_terminal = true;
        }
    }

    void apply_actions(
        typename MoldState::PlayerAction row_action, 
        typename MoldState::PlayerAction col_action
    ) {
        --this->depth;
        MoldState::transition_data.probability = 1.0;
        // For non-stochastic states (only one chance action), we use booleans to represent the probablity 8)
        MoldState::transition_data.chance_action = 0;
    }   
};
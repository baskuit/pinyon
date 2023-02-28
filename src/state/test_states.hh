#pragma once

#include "state.hh"

// Large uniform tree for testing etc. So called because it spreads out until it can't.

template <int size>
class MoldState : public StateArray<size, int, int, bool>
{
public:
    struct Types : StateArray<size, int, int, bool>::Types
    {
    };
    int depth;

    MoldState(int depth) : depth((depth >= 0) * depth) {}
    // MoldState (MoldState &t) {}

    void get_actions()
    {
        if (this->depth > 0)
        {
            for (int i = 0; i < size; ++i)
            {
                MoldState::actions.row_actions[i] = i;
                MoldState::actions.col_actions[i] = i;
            }
            MoldState::actions.rows = size;
            MoldState::actions.cols = size;
            this->is_terminal = false;
        }
        else
        {
            MoldState::actions.rows = 0;
            MoldState::actions.cols = 0;
            this->is_terminal = true;
        }
    }

    void apply_actions(
        typename MoldState::PlayerAction row_action,
        typename MoldState::PlayerAction col_action)
    {
        --this->depth;
        MoldState::transition_data.prob = true;
        // For deterministic and unknown prob states, we can be cheeky and use true/false for transtion probs
        MoldState::transition_data.obs = 0;
    }
};

class PennyMatching : public StateArray<2, int, int, bool>
{
public:
    struct Types : StateArray<2, int, int, double>::Types
    {
    };
    // Let's try using the surskit design pattern.

    // PennyMatching () {
    //     this->actions.rows = 2;
    //     this->actions.cols = 2;
    // }

    void get_actions()
    {
        this->actions.rows = 2;
        this->actions.cols = 2;
        for (int i = 0; i < 2; ++i)
        {
            this->actions.row_actions[i] = i;
            this->actions.col_actions[i] = i;
        }
    }
    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action // PennyMatching has a fixed Action type, but nevertheless we can pretend it doesn't too see how to handle things when they get less clear.
    )
    {
        this->transition.prob = true;
        this->transition.obs = 0;
        this->is_terminal = true;
        if (row_action == col_action)
        {
            this->row_payoff = 1.0;
            this->col_payoff = 0.0;
        }
        else
        {
            this->row_payoff = 0.0;
            this->col_payoff = 1.0;
        }
    }
};
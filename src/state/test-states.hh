#pragma once

#include <state/state.hh>

/*
 Large uniform tree for testing etc. So called because it grows until it can't.
*/

template <int size>
class MoldState : public State<SimpleTypes>
{
public:
    struct Types : State<SimpleTypes>::Types
    {
    };
    
    size_t max_depth = 1;

    MoldState(size_t max_depth) : max_depth((max_depth >= 0) * max_depth)
    {
        this->actions.row_actions.fill(size);
        this->actions.col_actions.fill(size);

        for (int i = 0; i < size; ++i)
        {
            this->actions.row_actions[i] = i;
            this->actions.col_actions[i] = i;
        }
        this->prob = 1.0;
        this->obs = 0;
    }

    void get_actions()
    {
        if (this->max_depth <= 0)
        {
            this->actions.rows = 0;
            this->actions.cols = 0;
            this->is_terminal = true;
        }
    }

    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action)
    {
        --this->max_depth;
    }
};
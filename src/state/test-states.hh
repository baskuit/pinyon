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

    static constexpr bool IS_CONSTANT_SUM = false;
    
    size_t max_depth = 1;

    MoldState(size_t max_depth) : max_depth((max_depth >= 0) * max_depth)
    {
        this->row_actions.fill(size);
        this->col_actions.fill(size);

        for (int i = 0; i < size; ++i)
        {
            this->row_actions[i] = i;
            this->col_actions[i] = i;
        }
        this->prob = typename Types::Probability{1};
        this->obs = 0;
    }

    void get_actions()
    {
        this->is_terminal = (this->max_depth <= 0);
    }

    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action)
    {
        --this->max_depth;
    }
};
#pragma once

#include "state/state.hh"
/*
SeedState contains just enough info to pseudo-randomly expand to a recursively solved game tree.

The actual state representing the solved tree will be the TreeState object,
which is simply a wrapper the MatrixNode/ChanceNode tree created by the Grow algorithm on the SeedState.
*/

template <int _size>
class SeedState : public StateArray<_size, int, int, double>
{
public:
    struct Types : StateArray<_size, int, int, double>::Types
    {
        static const int size = _size;
    };
    int depth_bound = 0;
    int rows = _size;
    int cols = _size;

    prng &device;

    // SeedState(prng &device, int depth_bound, int rows, int cols) : device(device), depth_bound(depth_bound), rows(rows), cols(cols) {}
    int (*depth_bound_func)(prng &, int) = nullptr;
    int (*actions_func)(prng &, int) = nullptr;

    SeedState(prng &device, int depth_bound, int rows, int cols) : device(device), depth_bound(depth_bound), rows(rows), cols(cols)
    {
        this->transition.prob = 1;
        this->transition.obs = 0;
        if (this->depth_bound_func == nullptr)
        {
            this->depth_bound_func = &(SeedState::dbf);
        }
        if (this->actions_func == nullptr)
        {
            this->actions_func = &(SeedState::af);
        }
    }

    void get_actions()
    {
        this->actions.rows = rows;
        this->actions.cols = cols;
        for (int i = 0; i < rows; ++i)
        {
            this->actions.row_actions[i] = i;
        };
        for (int j = 0; j < cols; ++j)
        {
            this->actions.col_actions[j] = j;
        };
    }

    void apply_actions(int row_action, int col_action)
    {
        this->depth_bound = (*depth_bound_func)(this->device, this->depth_bound);
        depth_bound *= depth_bound >= 0;
        if (depth_bound == 0)
        {
            this->row_payoff = this->device.random_int(2);
            this->col_payoff = 1.0 - this->row_payoff;
            this->rows = 0;
            this->cols = 0;
            this->is_terminal = true;
        } else {
            this->rows = (*this->actions_func)(this->device, this->rows);
            this->cols = (*this->actions_func)(this->device, this->cols);
            this->is_terminal = false;// TODO remove
        }
    }

    /*
    Defaults
    */
    static int dbf(prng &device, int depth)
    {
        return (depth - 1) * (depth >= 0);
    }
    static int af(prng &device, int n_actions)
    {
        return n_actions;
    }
};
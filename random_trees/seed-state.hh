#pragma once

#include "state/state.hh"
/*
SeedState contains just enough info to pseudo-randomly expand to a recursively solved game tree.

The actual state representing the solved tree will be the TreeState object,
which is simply a wrapper the MatrixNode/ChanceNode tree created by the Grow algorithm on the SeedState.
*/

template <size_t MaxActions, size_t MaxTransitions>
class SeedState : public StateChanceVector<MaxActions, int, int, double>
{
public:
    struct Types : StateChanceVector<MaxActions, int, int, double>::Types
    {
        static const int size = MaxActions;
    };

    prng device;

    int depth_bound = 0;
    int rows = MaxActions;
    int cols = MaxActions;
    int bias = 0; // tanh of sigsum of bias becomes the payoff when terminal
    typename Types::VectorReal transition_probs;
    typename Types::Real transition_threshold = 1 / MaxActions;

    int (*depth_bound_func)(prng &, int) = nullptr;
    int (*actions_func)(prng &, int) = nullptr;

    SeedState(prng &device, int depth_bound, int rows, int cols, int (*depth_bound_func)(prng &, int), int (*actions_func)(prng &, int) ) : 
        device(device), depth_bound(depth_bound), rows(rows), cols(cols), depth_bound_func(depth_bound_func), actions_func(actions_func)
    {
        // this->transition.prob = 1;
        // this->transition.obs = 0; TODO confirm don't need to initialize, transition object will never be used until after apply_actions
        transition_probs.fill(MaxTransitions);
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
        this->actions.row_actions.fill(rows);
        this->actions.col_actions.fill(cols);
        for (int i = 0; i < rows; ++i)
        {
            this->actions.row_actions[i] = i;
        };
        for (int j = 0; j < cols; ++j)
        {
            this->actions.col_actions[j] = j;
        };
    }

    void apply_actions(int row_action, int col_action, int obs) // now a StateChance object
    {
        this->depth_bound = (*depth_bound_func)(this->device, this->depth_bound);
        depth_bound *= depth_bound >= 0;
        if (depth_bound == 0)
        {
            this->is_terminal = true;
            const double sigsum_bias = (bias > 0) - (bias < 0);
            this->row_payoff = (sigsum_bias + 1) / 2;
            this->col_payoff = 1.0 - this->row_payoff;
            rows = 0;
            cols = 0;
            // change device based on obs
        } else {
            rows = (*this->actions_func)(device, rows);
            cols = (*this->actions_func)(device, cols);
            bias += device.randint(3) - 1; // adds -1, 0, or 1
            get_transition_probs(device, transition_probs);
        }
    }

    /*
    Defaults
    */

    static int dbf(prng &device, int depth, int obs)
    {
        return (depth - 1) * (depth >= 0);
    }
    static int af(prng &device, int n_actions, int obs)
    {
        return n_actions;
    }
    static void get_transition_probs (prng &device, typename Types::VectorReal &output) {
        typename Types::Real prob_sum = 0;
        for (int i = 0; i < MaxTransitions; ++i) { // does static function play well with Template param?
            const typename Types::Real p = device.uniform();
            output[i] = p;
            prob_sum += p;
        }
        typename Types::Real new_prob_sum = 0;
        for (int i = 0; i < MaxTransitions; ++i) {
            typename Types::Real &p = output[i];
            p /= prob_sum;
            if (p < transition_threshold) {
                p = 0;
            }
            new_prob_sum += p;
        }
        for (int i = 0; i < MaxTransitions; ++i) {
            output[i] /= new_prob_sum;
        }
    }
};
#pragma once

#include "state/state.hh"
#include "libsurskit/random.hh"

#include <vector>

/*
SeedState contains just enough info to pseudo-randomly expand to a recursively solved game tree.

The actual state representing the solved tree will be the TreeState object,
which is simply a wrapper the MatrixNode/ChanceNode tree created by the Grow algorithm on the SeedState.
*/

template <size_t MaxActions, size_t MaxTransitions>
class SeedState : public StateChanceArray<MaxActions, int, int, double>
{
public:
    struct Types : StateChanceArray<MaxActions, int, int, double>::Types
    {
        static const int size = MaxActions;
    };

    prng device;
    int depth_bound = 0;
    int rows = MaxActions;
    int cols = MaxActions;
    int payoff_bias = 0;
    std::array<typename Types::Probability, MaxTransitions> transition_probs;
    typename Types::Probability transition_threshold = Rational(1, MaxTransitions);

    int (*depth_bound_func)(prng &, int) = nullptr;
    int (*actions_func)(prng &, int) = nullptr;

    SeedState(
        prng &device, 
        int depth_bound, 
        int rows, 
        int cols, 
        int (*depth_bound_func)(prng &, int), 
        int (*actions_func)(prng &, int))
            : device(device), depth_bound(depth_bound), rows(rows), cols(cols), depth_bound_func(depth_bound_func), actions_func(actions_func)
    {
        if (this->depth_bound_func == nullptr)
        {
            this->depth_bound_func = &(SeedState::dbf);
        }
        if (this->actions_func == nullptr)
        {
            this->actions_func = &(SeedState::af);
        }
        get_transition_probs(device, transition_probs);
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

    void get_chance_actions (
        std::vector<typename Types::Observation> &chance_actions,
        typename Types::Action row_action,
        typename Types::Action col_action) 
    {
        chance_actions.clear();
        for (int chance_idx = 0; chance_idx < MaxTransitions; ++chance_idx) {
            if (transition_probs[chance_idx] > 0) {
                chance_actions.push_back(chance_idx);
            }
        }
    }

    void apply_actions(int row_action, int col_action, int obs) // now a StateChance object
    {
        device.discard(obs); // advance the prng
        depth_bound = (*depth_bound_func)(this->device, this->depth_bound);
        depth_bound *= depth_bound >= 0;
        payoff_bias += device.random_int(3) - 1; // adds -1, 0, or 1
        if (depth_bound == 0)
        {
            this->is_terminal = true;
            const double sigsum_bias = (payoff_bias > 0) - (payoff_bias < 0);
            this->row_payoff = (sigsum_bias + 1) / 2;
            this->col_payoff = 1.0 - this->row_payoff;
            rows = 0;
            cols = 0;
            // change device based on obs
        } else {
            rows = (*this->actions_func)(device, rows);
            cols = (*this->actions_func)(device, cols);
            get_transition_probs(device, transition_probs);
        }
    }

    // SeedState is only used to generate a TreeState, and the grow algorithm only calls the other apply_actions
    void apply_actions(int row_action, int col_action)
    {
        int obs = device.sample_pdf(transition_probs, MaxTransitions);
        apply_actions(row_action, col_action, obs);
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

// private:
    void get_transition_probs (prng &device, std::array<typename Types::Probability, MaxTransitions> &output) {
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
#pragma once

#include "state/state.hh"
#include "libsurskit/random.hh"

#include <vector>
#include <span>

/*
SeedState contains just enough info to pseudo-randomly expand to a recursively solved game tree.

The actual state representing the solved tree will be the TreeState object,
which is simply a wrapper the MatrixNode/ChanceNode tree created by the Grow algorithm on the SeedState.

Probabilty type can be changed to Rational once coversion from double is finished (TODO)
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
    std::array<typename Types::Probability, MaxActions * MaxActions * MaxTransitions> chance_strategies;
    std::array<typename Types::Probability, MaxTransitions> chance_strategy;
    typename Types::Probability chance_threshold = Rational(1, MaxTransitions);

    int (*depth_bound_func)(prng &, int) = &(SeedState::dbf);
    int (*actions_func)(prng &, int) = &(SeedState::af);
    int (*payoff_bias_func)(prng &, int) = &(SeedState::pbf);

    SeedState(
        const prng &device, 
        int depth_bound, 
        int rows, 
        int cols,
        typename Types::Probability chance_threshold) : 
            device(device), 
            depth_bound(depth_bound), 
            rows(rows), 
            cols(cols),
            chance_threshold(chance_threshold)
    {
        get_chance_strategies();
    }

    SeedState(
        const prng &device, 
        int depth_bound, 
        int rows, 
        int cols, 
        int (*depth_bound_func)(prng &, int), 
        int (*actions_func)(prng &, int),
        int (*payoff_bias_func)(prng &, int)) : 
            device(device), 
            depth_bound(depth_bound), 
            rows(rows), 
            cols(cols), 
            depth_bound_func(depth_bound_func), 
            actions_func(actions_func), 
            payoff_bias_func(payoff_bias_func)
    {
        get_chance_strategies();
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
        const int start_idx = get_transition_idx(row_action, col_action, 0);
        for (int chance_idx = 0; chance_idx < MaxTransitions; ++chance_idx) {
            if (chance_strategies[start_idx + chance_idx] > 0) {
                chance_actions.push_back(chance_idx);
            }
        }
    }

    void apply_actions(int row_action, int col_action, int chance_action, bool extra_prng_call = true)
    {
        if (extra_prng_call) {
            device.uniform(); // to mirror sampling from chance actions
        }
        const int transition_idx = get_transition_idx(row_action, col_action, chance_action);
        device.discard(transition_idx); 
        // advance the prng so that different player/chance actions have different outcomes

        this->transition.obs = chance_action;
        this->transition.prob = chance_strategies[transition_idx];

        depth_bound = (*depth_bound_func)(device, depth_bound);
        depth_bound *= depth_bound >= 0;
        payoff_bias = (*payoff_bias_func)(device, payoff_bias);

        if (depth_bound == 0)
        {
            this->is_terminal = true;
            const typename Types::Real sigsum_bias = (payoff_bias > 0) - (payoff_bias < 0);
            this->row_payoff = (sigsum_bias + 1) / 2;
            this->col_payoff = 1.0 - this->row_payoff;
        } else {
            rows = (*actions_func)(device, rows);
            cols = (*actions_func)(device, cols);
            get_chance_strategies();
        }
    }

    // SeedState is only used to generate a TreeState, and the grow algorithm only calls the other apply_actions
    void apply_actions(int row_action, int col_action)
    {
        const int transition_idx = get_transition_idx(row_action, col_action, 0);
        std::copy_n(
            chance_strategies.begin() + transition_idx, 
            MaxTransitions,
            chance_strategy.begin());
        int chance_action = device.sample_pdf(chance_strategy, MaxTransitions);
        apply_actions(row_action, col_action, chance_action, false);
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
    static int pbf(prng &device, int payoff_bias)
    {
        const int bias = 1;
        return payoff_bias + device.random_int(2 * bias + 1) - bias;
    }

private:

    inline int get_transition_idx (int row_idx, int col_idx, int chance_idx) {
        return row_idx * MaxActions * MaxTransitions + col_idx * MaxTransitions + chance_idx;
    }

    void get_chance_strategies () {
        
        for (int row_idx = 0; row_idx < rows; ++row_idx) {

            for (int col_idx = 0; col_idx < cols; ++col_idx) {

                int start_idx = row_idx * MaxActions * MaxTransitions + col_idx * MaxTransitions;

                // get unnormalized distro
                typename Types::Probability prob_sum = 0;
                for (int i = 0; i < MaxTransitions; ++i) { 
                    const typename Types::Probability p = device.uniform();
                    chance_strategies[start_idx + i] = p;
                    prob_sum += p;
                }
                
                // clip and compute new norm
                typename Types::Probability new_prob_sum = 0;
                for (int i = 0; i < MaxTransitions; ++i) {
                    typename Types::Probability &p = chance_strategies[start_idx + i];
                    p /= prob_sum;
                    if (p < chance_threshold) {
                        p = 0;
                    }
                    new_prob_sum += p;
                }

                // append final renormalized strategy
                for (int i = 0; i < MaxTransitions; ++i) {
                    chance_strategies[start_idx + i] /= new_prob_sum;
                }
            }
        }
    }
};
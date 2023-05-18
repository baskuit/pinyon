#pragma once

#include "../types/types.hh"
#include "state.hh"
#include "../libsurskit/random.hh"

#include <vector>
#include <span>

/*
RandomTree is a well-defined P-game.
*/

template <size_t MaxTransitions>
class RandomTree : public ChanceState<SimpleTypes>
{
public:
    struct Types : ChanceState<SimpleTypes>::Types
    {
    };

    typename Types::PRNG device;
    int depth_bound = 0;
    int rows = 0;
    int cols = 0;
    int payoff_bias = 0;
    typename Types::Probability chance_threshold = Rational<int>(1, MaxTransitions);
    std::vector<typename Types::Probability> chance_strategies;

    int (*depth_bound_func)(RandomTree *, int) = &(RandomTree::dbf);
    int (*actions_func)(RandomTree *, int) = &(RandomTree::af);
    int (*payoff_bias_func)(RandomTree *, int) = &(RandomTree::pbf);

    // everything above determines the abstract game tree exactly

    std::array<typename Types::Probability, MaxTransitions> chance_strategy;
    // just a helper for the sample_pdf function in apply_actions

    RandomTree(
        const typename Types::PRNG &device,
        int depth_bound,
        int rows,
        int cols,
        typename Types::Probability chance_threshold) : device(device),
                                                        depth_bound(depth_bound),
                                                        rows(rows),
                                                        cols(cols),
                                                        chance_threshold(chance_threshold)
    {
        get_chance_strategies();
    }

    RandomTree(
        const typename Types::PRNG &device,
        int depth_bound,
        int rows,
        int cols,
        int (*depth_bound_func)(typename Types::PRNG &, int),
        int (*actions_func)(typename Types::PRNG &, int),
        int (*payoff_bias_func)(typename Types::PRNG &, int)) : device(device),
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
        // TODO optimize? Init actions in constr and only update entries when row/col increases
        this->row_actions.fill(rows);
        this->col_actions.fill(cols);
        for (ActionIndex row_idx = 0; row_idx < rows; ++row_idx)
        {
            this->row_actions[row_idx] = row_idx;
        };
        for (ActionIndex col_idx = 0; col_idx < cols; ++col_idx)
        {
            this->col_actions[col_idx] = col_idx;
        };
    }

    void get_chance_actions(
        std::vector<typename Types::Observation> &chance_actions,
        typename Types::Action row_action,
        typename Types::Action col_action)
    {
        chance_actions.clear();
        const ActionIndex start_idx = get_transition_idx(row_action, col_action, 0);
        for (ActionIndex chance_idx = 0; chance_idx < MaxTransitions; ++chance_idx)
        {
            if (chance_strategies[start_idx + chance_idx] > 0)
            {
                chance_actions.push_back(chance_idx);
            }
        }
    }

    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action,
        typename Types::Observation chance_action,
        bool extra_prng_call = true)
    {
        if (extra_prng_call)
        {
            device.uniform(); // TODO check if discard 1 does the same.
        }
        const ActionIndex transition_idx = get_transition_idx(row_action, col_action, chance_action);
        device.discard(transition_idx);
        // advance the typename Types::PRNG so that different player/chance actions have different outcomes

        this->obs = chance_action;
        this->prob = chance_strategies[transition_idx];

        depth_bound = (*depth_bound_func)(this, depth_bound);
        depth_bound *= depth_bound >= 0;
        payoff_bias = (*payoff_bias_func)(this, payoff_bias);

        if (depth_bound == 0)
        {
            this->is_terminal = true;
            const typename Types::Real sigsum_bias = (payoff_bias > 0) - (payoff_bias < 0);
            this->row_payoff = (sigsum_bias + 1) / 2;
            this->col_payoff = 1.0 - this->row_payoff;
        }
        else
        {
            rows = (*actions_func)(this, rows);
            cols = (*actions_func)(this, cols);
            get_chance_strategies();
        }
    }

    // RandomTreeVector is only used to generate a TraversedState, and the grow algorithm only calls the other apply_actions
    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action)
    {
        const ActionIndex transition_idx = get_transition_idx(row_action, col_action, 0);
        std::copy_n(
            chance_strategies.begin() + transition_idx,
            MaxTransitions,
            chance_strategy.begin());
        const ActionIndex chance_action_idx = device.sample_pdf(chance_strategy, MaxTransitions);
        apply_actions(row_action, col_action, chance_action_idx, false); // TODO make type safe (assumes obs = int)
    }

    /*
    Default Growth Functions
    */

    static int dbf(RandomTree *state, int depth)
    {
        return (depth - 1) * (depth >= 0);
    }
    static int af(RandomTree *state, int n_actions)
    {
        return n_actions;
    }
    static int pbf(RandomTree *state, int payoff_bias)
    {
        const int bias = 1;
        return payoff_bias + state->device.random_int(2 * bias + 1) - bias;
    }

private:
    inline ActionIndex get_transition_idx(
        ActionIndex row_idx, 
        ActionIndex col_idx, 
        ActionIndex chance_idx)
    {
        return row_idx * cols * MaxTransitions + col_idx * MaxTransitions + chance_idx;
    }

    void get_chance_strategies()
    {
        chance_strategies.resize(rows * cols * MaxTransitions);
        for (ActionIndex row_idx = 0; row_idx < rows; ++row_idx)
        {

            for (ActionIndex col_idx = 0; col_idx < cols; ++col_idx)
            {

                ActionIndex start_idx = row_idx * cols * MaxTransitions + col_idx * MaxTransitions;

                // get unnormalized distro
                typename Types::Probability prob_sum = 0;
                for (ActionIndex chance_idx = 0; chance_idx < MaxTransitions; ++chance_idx)
                {
                    const typename Types::Probability p = device.uniform();
                    chance_strategies[start_idx + chance_idx] = p;
                    prob_sum += p;
                }

                // clip and compute new norm
                typename Types::Probability new_prob_sum = 0;
                for (ActionIndex chance_idx = 0; chance_idx < MaxTransitions; ++chance_idx)
                {
                    typename Types::Probability &p = chance_strategies[start_idx + chance_idx];
                    p /= prob_sum;
                    if (p < chance_threshold)
                    {
                        p = 0;
                    }
                    new_prob_sum += p;
                }

                // append final renormalized strategy
                for (ActionIndex chance_idx = 0; chance_idx < MaxTransitions; ++chance_idx)
                {
                    chance_strategies[start_idx + chance_idx] /= new_prob_sum;
                }
            }
        }
    }
};

#pragma once

#include <types/types.hh>
#include <state/state.hh>
#include <libsurskit/random.hh>

#include <vector>
#include <span>

/*
RandomTreeSerialized but with occasional serialized sections
*/

class RandomTreeSerialized : public ChanceState<SimpleTypes>
{
public:
    struct Types : ChanceState<SimpleTypes>::Types
    {
    };

    typename Types::PRNG device;
    int depth_bound = 0;
    size_t rows = 0;
    size_t cols = 0;
    size_t transitions = 1;
    int payoff_bias = 0;
    typename Types::Probability chance_threshold{typename Types::Rational(1, transitions + 1)};
    std::vector<typename Types::Probability> chance_strategies;

    typename Types::Probability serialized_chance{typename Types::Rational(1, 8)};
    size_t serialized_remaining = 0;
    size_t max_serialized = 4;
    bool is_row_choosing;
    bool is_row_advantage;
    bool is_row_action_first;
    bool is_col_action_first;

    void (*depth_bound_func)(RandomTreeSerialized *) = &(RandomTreeSerialized::dbf);
    void (*actions_func)(RandomTreeSerialized *) = &(RandomTreeSerialized::af);
    void (*payoff_bias_func)(RandomTreeSerialized *) = &(RandomTreeSerialized::pbf);

    // everything above determines the abstract game tree exactly

    std::vector<typename Types::Probability> chance_strategy;
    // just a helper for the sample_pdf function in apply_actions

    RandomTreeSerialized(
        const typename Types::PRNG &device,
        int depth_bound,
        size_t rows,
        size_t cols,
        size_t transitions,
        double chance_threshold)
        : device(device),
          depth_bound(depth_bound),
          rows(rows),
          cols(cols),
          transitions(transitions),
          chance_threshold(chance_threshold)
    {
        get_chance_strategies();
    }

    RandomTreeSerialized(
        const typename Types::PRNG &device,
        int depth_bound,
        size_t rows,
        size_t cols,
        size_t transitions,
        void (*depth_bound_func)(RandomTreeSerialized *),
        void (*actions_func)(RandomTreeSerialized *),
        void (*payoff_bias_func)(RandomTreeSerialized *))
        : device(device),
          depth_bound(depth_bound),
          rows(rows),
          cols(cols),
          transitions(transitions),
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
        for (ActionIndex chance_idx = 0; chance_idx < transitions; ++chance_idx)
        {
            if (chance_strategies[start_idx + chance_idx] > typename Types::Rational(0))
            {
                chance_actions.push_back(chance_idx);
            }
        }
    }

    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action,
        typename Types::Observation chance_action)
    {

        const ActionIndex transition_idx = get_transition_idx(row_action, col_action, chance_action);
        device.discard(transition_idx);
        // advance the typename Types::PRNG so that different player/chance actions have different outcomes

        this->obs = chance_action;
        this->prob = chance_strategies[transition_idx];

        (*depth_bound_func)(this);
        // depth_bound *= depth_bound >= 0;
        is_row_action_first = (row_action == row_actions[0]);
        is_col_action_first = (col_action == col_actions[0]);

        (*payoff_bias_func)(this);

        if (depth_bound == 0)
        {
            this->is_terminal = true;
            const typename Types::Real sigsum_bias{static_cast<double>((payoff_bias > 0) - (payoff_bias < 0))};
            this->row_payoff = static_cast<typename Types::Real>((sigsum_bias + 1.0) / 2.0);
            this->col_payoff = static_cast<typename Types::Real>(this->row_payoff * -1.0 + 1.0);
        }
        else
        {

            if (serialized_remaining == 0)
            {
                if (device.uniform_64() % 8 == 0)
                {
                    serialized_remaining = max_serialized;
                    is_row_advantage = device.uniform_64() % 2;
                }
            }

            (*actions_func)(this);
            get_chance_strategies();
        }
    }

    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action)
    {
        std::vector<typename Types::Observation> chance_actions{};
        get_chance_actions(chance_actions, row_action, col_action);
        typename Types::Observation chance_action = chance_actions[this->seed % chance_actions.size()];
        apply_actions(row_action, col_action, chance_action);
    }

    /*
    Default Growth Functions
    */

    static void dbf(RandomTreeSerialized *state)
    {
        const int depth = state->depth_bound;
        state->depth_bound = (depth - 1) * (depth >= 0);
    }
    static void af(RandomTreeSerialized *state)
    {
        if (state->serialized_remaining > 0)
        {
            size_t x = std::max(state->rows, state->cols);
            if (state->is_row_choosing)
            {
                state->rows = x;
                state->cols = 1;
            }
            else
            {
                state->rows = 1;
                state->cols = x;
            }
            state->is_row_choosing = !state->is_row_choosing;
            state->serialized_remaining--;
        }
    }
    static void pbf(RandomTreeSerialized *state)
    {
        const int bias = 1; // lmao

        if (state->serialized_remaining > 0)
        {
            if (state->is_row_advantage)
            {
                if (state->is_row_choosing)
                {
                    if (state->is_row_action_first)
                    {
                        state->payoff_bias += state->device.random_int(2 * bias + 1) - 0;
                    }
                    else
                    {
                        state->payoff_bias += state->device.random_int(2 * bias + 1) - 2;
                    }
                }
                else
                {
                    // do nothing?
                }
            }
            else
            {
                if (state->is_row_choosing)
                {
                    // do nothing?
                }
                else
                {
                    if (state->is_col_action_first)
                    {
                        state->payoff_bias += state->device.random_int(2 * bias + 1) - 2;
                    }
                    else
                    {
                        state->payoff_bias += state->device.random_int(2 * bias + 1) - 0;
                    }
                }
            }
        }
        else
        {
            state->payoff_bias += state->device.random_int(2 * bias + 1) - bias;
        }
    }

private:
    inline ActionIndex get_transition_idx(
        ActionIndex row_idx,
        ActionIndex col_idx,
        ActionIndex chance_idx)
    {
        return row_idx * cols * transitions + col_idx * transitions + chance_idx;
    }

    void get_chance_strategies()
    {
        chance_strategies.resize(rows * cols * transitions);
        for (ActionIndex row_idx = 0; row_idx < rows; ++row_idx)
        {

            for (ActionIndex col_idx = 0; col_idx < cols; ++col_idx)
            {

                ActionIndex start_idx = row_idx * cols * transitions + col_idx * transitions;

                // get unnormalized distro
                typename Types::Probability prob_sum{typename Types::Rational(0)};
                for (ActionIndex chance_idx = 0; chance_idx < transitions; ++chance_idx)
                {
                    const typename Types::Probability p{device.uniform()}; // Prob = double
                    chance_strategies[start_idx + chance_idx] = p;
                    prob_sum += p;
                }

                // clip and compute new norm
                typename Types::Probability new_prob_sum{typename Types::Rational(0)};
                for (ActionIndex chance_idx = 0; chance_idx < transitions; ++chance_idx)
                {
                    typename Types::Probability &p = chance_strategies[start_idx + chance_idx];
                    p /= prob_sum;
                    if (p < chance_threshold)
                    {
                        p = typename Types::Probability(typename Types::Rational(0));
                    }
                    new_prob_sum += p;
                }

                // append final renormalized strategy
                for (ActionIndex chance_idx = 0; chance_idx < transitions; ++chance_idx)
                {
                    chance_strategies[start_idx + chance_idx] /= new_prob_sum;
                }
            }
        }
    }
};

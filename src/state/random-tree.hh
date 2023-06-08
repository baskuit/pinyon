#pragma once

#include <types/types.hh>
#include <state/state.hh>
#include <types/random.hh>

#include <vector>
#include <iterator>

/*
RandomTree is a well-defined P-game.
*/

class RandomTree : public ChanceState<RandomTreeTypes>
{
public:
    struct Types : ChanceState<RandomTreeTypes>::Types
    {
    };

    typename Types::PRNG device;
    typename Types::Seed seed{};
    int depth_bound = 0;
    size_t rows = 0;
    size_t cols = 0;
    size_t transitions = 1;
    int payoff_bias = 0;
    typename Types::Probability chance_threshold{typename Types::Rational(1, transitions + 1)};
    std::vector<typename Types::Probability> chance_strategies;

    int (*depth_bound_func)(RandomTree *, int) = &(RandomTree::dbf);
    int (*actions_func)(RandomTree *, int) = &(RandomTree::af);
    int (*payoff_bias_func)(RandomTree *, int) = &(RandomTree::pbf);

    // everything above determines the abstract game tree exactly

    std::vector<typename Types::Probability> chance_strategy;
    // just a helper for the sample_pdf function in apply_actions

    RandomTree(
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

    RandomTree(
        const typename Types::PRNG &device,
        int depth_bound,
        size_t rows,
        size_t cols,
        size_t transitions,
        int (*depth_bound_func)(RandomTree *, int),
        int (*actions_func)(RandomTree *, int),
        int (*payoff_bias_func)(RandomTree *, int))
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

    void reseed(typename Types::Seed seed)
    {
        this->seed = seed;
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

        depth_bound = (*depth_bound_func)(this, depth_bound);
        depth_bound *= depth_bound >= 0;
        payoff_bias = (*payoff_bias_func)(this, payoff_bias);

        if (depth_bound == 0)
        {
            this->is_terminal = true;
            const typename Types::Real sigsum_bias{static_cast<double>((payoff_bias > 0) - (payoff_bias < 0))};
            this->payoff.row_value = static_cast<typename Types::Real>((sigsum_bias + 1.0) / 2.0);
        }
        else
        {
            rows = (*actions_func)(this, rows);
            cols = (*actions_func)(this, cols);
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

/*

Helper class to generate random tree instances for testing

*/

class RandomTreeGenerator
{
public:
    prng device;
    std::vector<size_t> depth_bound_vec;
    std::vector<size_t> actions_vec;
    std::vector<size_t> chance_action_vec;
    std::vector<double> chance_threshold_vec;
    size_t trials;

    std::array<size_t, 5> sizes;
    size_t length;

    // template <typename Float>
    RandomTreeGenerator(
        prng &device,
        std::vector<size_t> &depth_bound_vec,
        std::vector<size_t> &actions_vec,
        std::vector<size_t> &chance_action_vec,
        std::vector<double> &chance_threshold_vec,
        size_t trials)
        : device{device},
          depth_bound_vec{depth_bound_vec},
          actions_vec{actions_vec},
          chance_action_vec{chance_action_vec},
          chance_threshold_vec{chance_threshold_vec},
          trials{trials}
    {
        length =
            depth_bound_vec.size() *
            actions_vec.size() *
            chance_action_vec.size() *
            chance_threshold_vec.size() *
            trials;
        sizes = {
            trials - 1,
            chance_threshold_vec.size() - 1,
            chance_action_vec.size() - 1,
            actions_vec.size() - 1,
            depth_bound_vec.size() - 1,
        };
    }

    struct Iterator
    {
        RandomTreeGenerator *ptr;
        std::array<size_t, 5> params_indices{0};
        Iterator(RandomTreeGenerator *ptr, std::array<size_t, 5> params_indices) 
        : ptr{ptr}, params_indices{params_indices} {}

        RandomTree operator*()
        {
            return ptr->get_state(params_indices);
        }
        Iterator &operator++()
        {
            ++params_indices[0];
            for (int i = 0; i < 4; ++i)
            {
                if (params_indices[i] > ptr->sizes[i])
                {
                    params_indices[i] = 0;
                    ++params_indices[i + 1];
                }
            }
            return *this;
        }
        bool operator==(const Iterator &other)
        {
            return ptr == other.ptr && params_indices == other.params_indices;
        }
    };

    Iterator begin()
    {
        return Iterator{this, {0}};
    }

    Iterator end()
    {
        auto sizes_ = sizes;
        sizes_[4]++; 
        return Iterator{this, sizes};
    }

    RandomTree get_state(std::array<size_t, 5> &params_indices)
    {

        uint64_t seed = device.uniform_64();

        return RandomTree{
            prng{seed}, 
            depth_bound_vec[params_indices[4]],
            actions_vec[params_indices[3]],
            actions_vec[params_indices[3]],
            chance_action_vec[params_indices[2]],
            chance_threshold_vec[params_indices[1]]
        };
    } // TODO still off by one error
};
#pragma once

#include <types/types.hh>
#include <state/state.hh>
#include <types/random.hh>

#include <vector>
#include <iterator>
#include <algorithm>
#include <cstdio>
#include <iterator>
#include <ranges>
#include <string>

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

struct RandomTreeGenerator
{

    using Tuple = std::tuple<size_t, size_t, size_t, double>;

    prng device;
    uint64_t seed;
    const std::vector<size_t> depth_bound_vec;
    const std::vector<size_t> actions_vec;
    const std::vector<size_t> chance_action_vec;
    const std::vector<double> chance_threshold_vec;
    const size_t trials;

    using Product = decltype(std::views::cartesian_product(depth_bound_vec, actions_vec, chance_action_vec, chance_threshold_vec));
    using It = std::ranges::iterator_t<Product>;

    Product view = std::views::cartesian_product(depth_bound_vec, actions_vec, chance_action_vec, chance_threshold_vec);

    RandomTreeGenerator(
        prng &d,
        std::vector<size_t> &depth_bound_vec,
        std::vector<size_t> &actions_vec,
        std::vector<size_t> &chance_action_vec,
        std::vector<double> &chance_threshold_vec,
        size_t trials)
        : device{d},
          depth_bound_vec{depth_bound_vec},
          actions_vec{actions_vec},
          chance_action_vec{chance_action_vec},
          chance_threshold_vec{chance_threshold_vec},
          trials{trials}
    {
        seed = device.uniform_64();
    }

    class Iterator : public It
    {
    public:
        RandomTreeGenerator *ptr;
        size_t trial = 0;

        Iterator(const It &it, RandomTreeGenerator *ptr) : It{it}, ptr{ptr}
        {
        }

        Iterator &operator++()
        {
            if (trial == 0) {
                It::operator++();
            }
            ptr->seed = ptr->device.uniform_64();

            ++trial;
            trial %= ptr->trials;

            return (*this);
        }

        RandomTree operator*()
        {

            Tuple tuple = It::operator*();
            std::cout << std::get<0>(tuple) << ' ' << std::get<1>(tuple) << ' ' << std::get<2>(tuple) << ' ' << std::get<3>(tuple) << std::endl;

            return RandomTree{
                prng{ptr->seed},
                static_cast<int>(std::get<0>(tuple)),
                std::get<1>(tuple),
                std::get<1>(tuple),
                std::get<2>(tuple),
                std::get<3>(tuple)};
        }

        bool operator==(const Iterator &other) const
        {
            return static_cast<const It &>(*this) == static_cast<const It &>(other) && trial == other.trial;
        }
    };

    Iterator begin()
    {
        return Iterator(view.begin(), this);
    }

    Iterator end()
    {
        return Iterator(view.end(), this);
    }
};
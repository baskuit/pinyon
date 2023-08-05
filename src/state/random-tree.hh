#pragma once

#include <libsurskit/generator.hh>
#include <types/types.hh>
#include <types/random.hh>
#include <state/state.hh>
#include <wrapper/basic.hh>

#include <vector>
#include <ranges>

template <IsTypeList Types = RandomTreeFloatTypes>
struct RandomTree : Types
{
    class State : public PerfectInfoState<Types>
    {
    public:
        Types::PRNG device{};
        Types::Seed transition_seed{};
        size_t depth_bound = 0;
        size_t rows = 0;
        size_t cols = 0;
        const size_t transitions = 1;
        int payoff_bias = 0;
        const typename Types::Q chance_threshold{1, static_cast<int>(transitions + 1)};
        std::vector<typename Types::Prob> chance_strategies;
        int chance_denominator = 10;

        int (*depth_bound_func)(State *, int) = &(State::depth_bound_default);
        int (*actions_func)(State *, int) = &(State::actions_default);
        int (*payoff_bias_func)(State *, int) = &(State::payoff_bias_default);

        // everything above determines the abstract game tree exactly

        std::vector<typename Types::Prob> chance_strategy;
        // just a helper for the sample_pdf function in apply_actions

        State () {}

        State(
            const Types::PRNG &device,
            size_t depth_bound,
            size_t rows,
            size_t cols,
            size_t transitions,
            const Types::Q &chance_threshold)
            : device{device},
              depth_bound{depth_bound},
              rows{rows},
              cols{cols},
              transitions{transitions},
              chance_threshold{chance_threshold}
        {
            get_chance_strategies();
        }

        State(
            const Types::PRNG &device,
            size_t depth_bound,
            size_t rows,
            size_t cols,
            size_t transitions,
            Types::Q chance_threshold,
            int (*depth_bound_func)(State *, int),
            int (*actions_func)(State *, int),
            int (*payoff_bias_func)(State *, int))
            : device{device},
              depth_bound{depth_bound},
              rows{rows},
              cols{cols},
              transitions{transitions},
              chance_threshold{chance_threshold},
              depth_bound_func{depth_bound_func},
              actions_func{actions_func},
              payoff_bias_func{payoff_bias_func}
        {
            get_chance_strategies();
        }

        void randomize_transition(Types::PRNG &device)
        {
            transition_seed = device.uniform_64();
        }

        void get_actions()
        {
            this->row_actions.resize(rows);
            this->col_actions.resize(cols);
            for (ActionIndex row_idx = 0; row_idx < rows; ++row_idx)
            {
                this->row_actions[row_idx] = typename Types::Action{row_idx};
            };
            for (ActionIndex col_idx = 0; col_idx < cols; ++col_idx)
            {
                this->col_actions[col_idx] = typename Types::Action{col_idx};
            };
        }

        void get_actions(
            Types::VectorAction &row_actions,
            Types::VectorAction &col_actions) const
        {
            row_actions.resize(rows);
            col_actions.resize(cols);
            for (ActionIndex row_idx = 0; row_idx < rows; ++row_idx)
            {
                row_actions[row_idx] = typename Types::Action{row_idx};
            };
            for (ActionIndex col_idx = 0; col_idx < cols; ++col_idx)
            {
                col_actions[col_idx] = typename Types::Action{col_idx};
            };
        }

        void get_chance_actions(
            std::vector<typename Types::Obs> &chance_actions,
            const Types::Action row_action,
            const Types::Action col_action) const
        {
            chance_actions.clear();
            const size_t start_idx = get_transition_idx(row_action, col_action, typename Types::Obs{0});
            for (ActionIndex chance_idx = 0; chance_idx < transitions; ++chance_idx)
            {
                if (chance_strategies[start_idx + chance_idx] > typename Types::Prob{0})
                {
                    chance_actions.push_back(typename Types::Obs{chance_idx});
                }
            }
        }

        void apply_actions(
            Types::Action row_action,
            Types::Action col_action,
            Types::Obs chance_action)
        {

            const ActionIndex transition_idx = get_transition_idx(row_action, col_action, chance_action);
            device.discard(transition_idx);
            // advance the Types::PRNG so that different player/chance actions have different outcomes

            this->obs = chance_action;
            this->prob = chance_strategies[transition_idx];

            depth_bound = (*depth_bound_func)(this, depth_bound);
            depth_bound *= depth_bound >= 0;
            payoff_bias = (*payoff_bias_func)(this, payoff_bias);

            if (depth_bound == 0)
            {
                this->terminal = true;
                typename Types::Q row_payoff{(payoff_bias > 0) - (payoff_bias < 0) + 1, 2};
                row_payoff.canonicalize();
                this->payoff = typename Types::Value{typename Types::Real{row_payoff}};
            }
            else
            {
                rows = (*actions_func)(this, rows);
                cols = (*actions_func)(this, cols);
                get_chance_strategies();
            }
        }

        void apply_actions(
            Types::Action row_action,
            Types::Action col_action)
        {
            std::vector<typename Types::Obs> chance_actions{};
            get_chance_actions(chance_actions, row_action, col_action);
            typename Types::Obs chance_action = chance_actions[this->transition_seed % chance_actions.size()];
            apply_actions(row_action, col_action, chance_action);
        }

        /*
        Default Growth Functions
        */

        static int depth_bound_default(State *state, int depth)
        {
            return (depth - 1) * (depth >= 0);
        }
        static int actions_default(State *state, int n_actions)
        {
            return n_actions;
        }
        static int payoff_bias_default(State *state, int payoff_bias)
        {
            const int bias = 1;
            return payoff_bias + state->device.random_int(2 * bias + 1) - bias;
        }

    private:
        inline size_t get_transition_idx(
            Types::Action row_action,
            Types::Action col_action,
            Types::Obs chance_action) const
        {
            const ActionIndex row_idx{row_action};
            const ActionIndex col_idx{col_action};
            const ActionIndex chance_idx{chance_action};
            return row_idx * cols * transitions + col_idx * transitions + chance_idx;
        }

        void get_chance_strategies()
        {
            typename Types::template Vector<typename Types::Q> chance_strategies_;
            chance_strategies_.resize(rows * cols * transitions);
            chance_strategies.resize(rows * cols * transitions);
            for (ActionIndex row_idx = 0; row_idx < rows; ++row_idx)
            {

                for (ActionIndex col_idx = 0; col_idx < cols; ++col_idx)
                {

                    ActionIndex start_idx = row_idx * cols * transitions + col_idx * transitions;

                    // get unnormalized distro
                    typename Types::Q prob_sum{typename Types::Q(0)};
                    for (ActionIndex chance_idx = 0; chance_idx < transitions; ++chance_idx)
                    {
                        const int num = device.random_int(chance_denominator) + 1;

                        typename Types::Q x{num, chance_denominator};
                        if (x < chance_threshold)
                        {
                            x = Rational<>{0};
                        }
                        chance_strategies_[start_idx + chance_idx] = x;
                        prob_sum += x;
                    }

                    if (prob_sum == typename Types::Q{0})
                    {
                        chance_strategies_[start_idx] = typename Types::Q{1};
                        prob_sum = typename Types::Q{1};
                    }

                    for (ActionIndex chance_idx = 0; chance_idx < transitions; ++chance_idx)
                    {
                        auto &x = chance_strategies_[start_idx + chance_idx];
                        x = x / prob_sum; // reduced here
                        chance_strategies[start_idx + chance_idx] = typename Types::Prob{x};
                    }
                }
            }
        }
    };
};

/*

Helper class to generate random tree instances for testing

*/

template <typename TypeList = RandomTreeFloatTypes>
struct RandomTreeGenerator : CartesianProductGenerator<W::StateWrapper<RandomTree<TypeList>>, std::vector<size_t>, std::vector<size_t>, std::vector<size_t>, std::vector<Rational<>>, std::vector<size_t>>
{
    inline static prng device{};

    static W::StateWrapper<RandomTree<TypeList>> constr(std::tuple<size_t, size_t, size_t, Rational<>, size_t> tuple) // static otherwise implcit this arg messes up signature
    {
        return W::StateWrapper<RandomTree<TypeList>>{
            RandomTreeGenerator::device.uniform_64(),
            static_cast<int>(std::get<0>(tuple)),
            std::get<1>(tuple),
            std::get<1>(tuple),
            std::get<2>(tuple),
            std::get<3>(tuple)};
    };

    RandomTreeGenerator(
        prng device,
        std::vector<size_t> depth_bound_vec,
        std::vector<size_t> actions_vec,
        std::vector<size_t> chance_action_vec,
        std::vector<Rational<>> chance_threshold_vec,
        std::vector<size_t> trial_vec)
        : CartesianProductGenerator<W::StateWrapper<RandomTree<TypeList>>, std::vector<size_t>, std::vector<size_t>, std::vector<size_t>, std::vector<Rational<>>, std::vector<size_t>>{
              constr, depth_bound_vec, actions_vec, chance_action_vec, chance_threshold_vec, trial_vec}
    {
        RandomTreeGenerator::device = prng{device};
    }
};

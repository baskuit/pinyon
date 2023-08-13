#pragma once

#include <state/state.hh>
#include <model/model.hh>

template <CONCEPT(IsPerfectInfoStateTypes, Types), bool HasPolicy = false>
struct MonteCarloModel : Types
{
    struct Inference
    {
        typename Types::Value value;
        typename Types::VectorReal row_policy, col_policy;
    };
    using ModelInput = typename Types::State;
    using ModelOutput = Inference;
    using ModelBatchInput = std::vector<ModelInput>;
    using ModelBatchOutput = std::vector<ModelOutput>;

    class Model
    {
    public:
        typename Types::PRNG device{};

        Model() {}

        Model(const Types::PRNG &device) : device{device} {}
        
        void get_input(
            const Types::State &state,
            ModelInput &input)
        {
            input = state;
            typename Types::State s;
        }

        void get_batch_input(
            const std::vector<typename Types::State> &states,
            ModelBatchInput &inputs)
        {
            inputs = states;
        }

        void get_inference(
            ModelInput &input,
            ModelOutput &output)
        {
            if constexpr (HasPolicy)
            {
                const typename Types::Real row_uniform{Rational{1, static_cast<int>(input.row_actions.size())}};
                output.row_policy.resize(input.row_actions.size(), row_uniform);
                const typename Types::Real col_uniform{Rational{1, static_cast<int>(input.col_actions.size())}};
                output.col_policy.resize(input.col_actions.size(), col_uniform);
            }
            rollout(input);
            output.value = input.payoff;
        }

        void get_inference(
            ModelBatchInput &inputs,
            ModelBatchOutput &outputs)
        {
            outputs.resize(inputs.size());
            for (int i = 0; i < inputs.size(); ++i)
            {
                auto &input = inputs[i];
                auto &output = outputs[i];
                get_inference(input, output);
            }
        }

        void get_value(
            ModelInput &input,
            Types::Value &value)
        {
            rollout(input);
            value = input.payoff;
        }

        void add_to_batch_input(
            Types::State &state,
            ModelBatchInput &input)
        {
            input.push_back(state);
        }

    protected:
        void rollout(typename Types::State &state)
        {
            while (!state.is_terminal())
            {
                const ActionIndex row_idx = device.random_int(state.row_actions.size());
                const ActionIndex col_idx = device.random_int(state.col_actions.size());
                const typename Types::Action row_action{state.row_actions[row_idx]};
                const typename Types::Action col_action{state.col_actions[col_idx]};
                state.apply_actions(row_action, col_action);
                state.get_actions();
            }
        }
    };

    // if constexpr (HasPolicy)
    // {
    //     static_assert(IsValuePolicyModelTypes < MonteCarloModel < Types >>>);
    // }
    // else
    // {
    //     static_assert(IsValueModelTypes < MonteCarloModel < Types >>>);
    // }
};
#pragma once

#include <state/state.hh>
#include <model/model.hh>

template <CONCEPT(IsPerfectInfoStateTypes, Types), bool HasPolicy = false>
struct MonteCarloModel : Types
{
    using ModelInput = typename Types::State;
    struct ModelOutput
    {
        typename Types::Value value;
        typename Types::VectorReal row_policy, col_policy;
    };
    using ModelBatchInput = std::vector<ModelInput>;
    using ModelBatchOutput = std::vector<ModelOutput>;

    class Model
    {
    public:
        typename Types::PRNG device;

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

        void inference(
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

        void inference(
            ModelBatchInput &inputs,
            ModelBatchOutput &outputs)
        {
            outputs.resize(inputs.size());
            for (int i = 0; i < inputs.size(); ++i)
            {
                auto &input = inputs[i];
                auto &output = outputs[i];
                inference(input, output);
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
        void rollout(Types::State &state)
        {
            while (!state.is_terminal())
            {
                const int row_idx = device.random_int(state.row_actions.size());
                const int col_idx = device.random_int(state.col_actions.size());
                const auto row_action = state.row_actions[row_idx];
                const auto col_action = state.col_actions[col_idx];
                state.apply_actions(row_action, col_action);
                state.get_actions();
            }
        }
    };
};

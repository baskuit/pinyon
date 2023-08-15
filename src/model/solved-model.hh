#pragma once

#include <state/state.hh>

template <CONCEPT(IsSolvedStateTypes, Types)>
struct SolvedStateModel : Types
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
        void get_input(
            const Types::State &state,
            ModelInput &input) const
        {
            input = state;
        }

        void get_batch_input(
            const std::vector<typename Types::State> &states,
            ModelBatchInput &inputs) const
        {
            inputs = states;
        }

        void get_inference(
            ModelInput &input,
            ModelOutput &output)
        {
            output.value = input.get_payoff();
            input.get_strategies(output.row_policy, output.col_policy);
            // std::cout << "printing matrix" << std::endl;
        }

        void get_inference(
            ModelBatchInput &inputs,
            ModelBatchOutput &outputs)
        {
            for (auto &output : outputs)
            {
                output.value = typename Types::Value{.5, .5};
            }
        }

        void get_value(
            ModelInput &input,
            Types::Value &value)
        {
            value = input.get_payoff();
        }

        void add_to_batch_input(
            Types::State &state,
            ModelBatchInput &input)
        {
            input.push_back(state);
        }
    };
};

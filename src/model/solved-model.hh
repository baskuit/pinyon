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
        void inference(
            ModelInput &input,
            ModelOutput &output)
        {
            output.value = input.get_payoff();
            input.get_strategies(output.row_policy, output.col_policy);
        }

        void inference(
            ModelBatchInput &batch_input,
            ModelBatchOutput &batch_output)
        {
            for (auto &output : batch_output)
            {
                output.value = typename Types::Value{.5, .5};
            }
        }

        void add_to_batch_input(
            Types::State &&state,
            ModelBatchInput &batch_input) const
        {
            batch_input.push_back(state);
        }
    };
};

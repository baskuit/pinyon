#pragma once

#include <state/state.hh>
#include <model/model.hh>

template <CONCEPT(IsPerfectInfoStateTypes, Types), bool HasPolicy = false>
struct MonteCarloModel : Types
{

    struct ModelOutput
    {
        typename Types::Value value;
        typename Types::VectorReal row_policy, col_policy;
    };
    using ModelBatchInput = std::vector<typename Types::State>;
    using ModelBatchOutput = std::vector<ModelOutput>;

    class Model
    {
    public:
        typename Types::PRNG device;

        Model(const Types::PRNG &device) : device{device} {}
        
        void inference(
            Types::State &&state,
            ModelOutput &output)
        {
            if constexpr (HasPolicy)
            {
                const size_t rows = state.row_actions.size();
                const size_t cols = state.col_actions.size();
                const typename Types::Real row_uniform{Rational{1, static_cast<int>(rows)}};
                output.row_policy.resize(rows, row_uniform);
                const typename Types::Real col_uniform{Rational{1, static_cast<int>(cols)}};
                output.col_policy.resize(cols, col_uniform);
            }
            rollout(state);
            output.value = state.payoff;
        }

        void inference(
            ModelBatchInput &batch_input,
            ModelBatchOutput &batch_output)
        {
            batch_output.resize(batch_input.size());
            for (int i = 0; i < batch_input.size(); ++i)
            {
                inference(std::move(batch_input[i]), batch_output[i]);
            }
        }

        void add_to_batch_input(
            Types::State &&state,
            ModelBatchInput &input) const
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

#pragma once

#include <state/state.hh>

#include <vector>

template <typename Types>
concept IsValueModelTypes =
    requires(
        typename Types::Model &model,
        typename Types::ModelInput &input,
        typename Types::ModelOutput &output,
        typename Types::State &state) {
        {
            output.value
        } -> std::same_as<typename Types::Value &>;
        {
            model.inference(
                input,
                output)
        } -> std::same_as<void>;
        {
            model.get_input(
                state,
                input)
        } -> std::same_as<void>;
    } &&
    IsPerfectInfoStateTypes<Types>; // this basically enforces that everything is perfect info (has Obs, Prob, data members)

template <typename Types>
concept IsBatchValueModelTypes =
    requires(
        typename Types::State &state,
        typename Types::Model &model,
        typename Types::ModelBatchInput &model_batch_input,
        typename Types::ModelBatchOutput &model_batch_output) {
        {
            model.inference(model_batch_input, model_batch_output)
        } -> std::same_as<void>;
        {
            model.add_to_batch_input(state, model_batch_input)
        } -> std::same_as<void>;
        {
            model_batch_input[0]
        } -> std::same_as<typename Types::ModelInput &>;
        {
            model_batch_output[0]
        } -> std::same_as<typename Types::ModelOutput &>;
    } &&
    IsValueModelTypes<Types>;

template <typename Types>
concept IsDoubleOracleModelTypes =
    requires(
        typename Types::Model &model,
        typename Types::ModelInput &input,
        typename Types::ModelOutput &output,
        typename Types::State &state) {
        {
            output.row_policy
        } -> std::same_as<typename Types::VectorReal &>;
        {
            output.col_policy
        } -> std::same_as<typename Types::VectorReal &>;
    } &&
    IsValueModelTypes<Types>;

template <CONCEPT(IsStateTypes, Types)>
struct EmptyModel : Types
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
            ModelInput &input)
        {
            input = state;
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
            const typename Types::Real row_uniform{Rational{1, static_cast<int>(input.row_actions.size())}};
            output.row_policy.resize(input.row_actions.size(), row_uniform);
            const typename Types::Real col_uniform{Rational{1, static_cast<int>(input.col_actions.size())}};
            output.col_policy.resize(input.col_actions.size(), col_uniform);

            if constexpr (Types::Value::IS_CONSTANT_SUM == true)
            {
                output.value = typename Types::Value{typename Types::Q{1, 2}};
            }
            else
            {
                output.value = typename Types::Value{typename Types::Q{1, 2}, typename Types::Q{1, 2}};
            }
        }

        void inference(
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
            if constexpr (Types::Value::IS_CONSTANT_SUM == true)
            {
                value = typename Types::Value{typename Types::Q{1, 2}};
            }
            else
            {
                value = typename Types::Value{typename Types::Q{1, 2}, typename Types::Q{1, 2}};
            }
        }

        void add_to_batch_input(
            Types::State &state,
            ModelBatchInput &input)
        {
            input.push_back(state);
        }
    };
};

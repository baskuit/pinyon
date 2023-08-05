#pragma once

#include <state/state.hh>

#include <vector>

template <template <typename> typename _Model, typename Types> 
struct Types_Model : Types {
    using ModelTypes = Types;
};

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
            model.get_inference(
                input,
                output)
        } -> std::same_as<void>;
        {
            model.get_input(
                state,
                input)
        } -> std::same_as<void>;
    } &&
    IsStateTypes<Types>;

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

template <IsStateTypes Types>
struct EmptyModel : Types
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

        void get_inference(
            ModelInput &input,
            ModelOutput &output)
        {
            output.value = typename Types::Value{typename Types::Q{1, 2}, typename Types::Q{1, 2}};
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
            value = typename Types::Value{typename Types::Q{1, 2}, typename Types::Q{1, 2}};
        }

        void add_to_batch_input(
            Types::State &state,
            ModelBatchInput &input)
        {
            input.push_back(state);
        }
    };
};

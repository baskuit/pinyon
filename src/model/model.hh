#pragma once

#include <state/state.hh>

#include <vector>

template <typename Types>
concept IsValueModelTypes =
    requires(
        typename Types::State &&moved_state,
        typename Types::Model &model,
        typename Types::ModelOutput &output) {
        {
            model.inference(std::forward<typename Types::State>(moved_state), output)
        } -> std::same_as<void>;
        {
            output.value
        } -> std::same_as<typename Types::Value &>;
    } &&
    IsPerfectInfoStateTypes<Types>;

template <typename Types>
concept IsBatchModelTypes =
    requires(
        const typename Types::State &const_state,
        typename Types::State &&moved_state,
        typename Types::Model &model,
        typename Types::ModelOutput &model_output,
        typename Types::ModelBatchInput &model_batch_input,
        typename Types::ModelBatchOutput &model_batch_output,
        typename Types::Mask &mask) {
        {
            model.inference(model_batch_input, model_batch_output)
        } -> std::same_as<void>;
        {
            model.add_to_batch_input(std::forward<typename Types::State>(moved_state), model_batch_input)
        } -> std::same_as<void>;
        {
            model.get_mask(mask, const_state)
        } -> std::same_as<void>;
        {
            model.get_output(model_output, model_batch_output, 0, mask)
        } -> std::same_as<void>;
    } &&
    IsValueModelTypes<Types>;

template <typename Types>
concept IsPolicyModelTypes =
    requires(
        typename Types::ModelOutput &output) {
        {
            output.row_policy
        } -> std::same_as<typename Types::VectorReal &>;
        {
            output.col_policy
        } -> std::same_as<typename Types::VectorReal &>;
    } &&
    IsValueModelTypes<Types>;

// Empty Model

template <CONCEPT(IsStateTypes, Types)>
struct EmptyModel : Types
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
        void inference(
            Types::State &&state,
            ModelOutput &output) const
        {
            const size_t rows = state.row_actions.size();
            const size_t cols = state.col_actions.size();
            const typename Types::Real row_uniform{Rational{1, static_cast<int>(rows)}};
            output.row_policy.resize(rows, row_uniform);
            const typename Types::Real col_uniform{Rational{1, static_cast<int>(cols)}};
            output.col_policy.resize(cols, col_uniform);
            output.value = make_draw<Types>();
        }

        void inference(
            ModelBatchInput &inputs,
            ModelBatchOutput &outputs) const
        {
            for (size_t i = 0; i < inputs.size(); ++i)
            {
                inference(typename Types::State{inputs[i]}, outputs[i]);
            }
        }

        void add_to_batch_input(
            Types::State &&state,
            ModelBatchInput &input) const
        {
            input.push_back(state);
        }
    };
};

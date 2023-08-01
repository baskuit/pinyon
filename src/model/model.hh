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
class EmptyModel
{
public:
    using ModelInput = typename Types::State;
    struct ModelOutput
    {
        typename Types::Value value;
    };
    using ModelBatchInput = std::vector<ModelInput>;
    using ModelBatchOutput = std::vector<ModelOutput>;
    struct T : Types {
        using Model = EmptyModel;
        using EmptyModel::ModelInput;
        using EmptyModel::ModelOutput;
        using EmptyModel::ModelBatchInput;
        using EmptyModel::ModelBatchOutput;
    };

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
        output.value = typename Types::Value{typename Types::Rational{1, 2}, typename Types::Rational{1, 2}};
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
};

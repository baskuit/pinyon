#pragma once

#include <state/state.hh>

#include <vector>

template <class _State>
class AbstractModel
{
public:
    struct Types : _State::Types
    {
        using State = _State;
    };
};

/*
Similar to `State`, in that virtually all models will be derived from it.
*/

template <class _State>

class DoubleOracleModel : public AbstractModel<_State>
{
    static_assert(std::derived_from<_State, State<typename _State::Types::TypeList>>);

public:
    struct ModelOutput;
    struct ModelBatchOutput
    {
    };
    struct ModelInput;
    struct ModelBatchInput;

    struct Types : AbstractModel<_State>::Types
    {
        using ModelOutput = DoubleOracleModel::ModelOutput;
    };

    struct ModelOutput
    {
        typename Types::Real row_value;
        typename Types::Real col_value;
        typename Types::VectorReal row_policy;
        typename Types::VectorReal col_policy;
    };

    void get_input(
        const typename Types::State &state,
        ModelInput &input);

    void get_batch_input(
        const std::vector<typename Types::State> &states,
        ModelBatchInput &inputs);

    void get_inference(
        ModelInput &input,
        ModelOutput &output);

    void get_inference(
        ModelBatchInput &inputs,
        ModelBatchOutput &outputs);
};

/*
Universal model.
*/

template <class _State>
class MonteCarloModel : public DoubleOracleModel<_State>
{
    // static_assert(std::derived_from<_State, _State<typename State::Types::TypeList>>);

public:
    struct Types : DoubleOracleModel<_State>::Types
    {
        using ModelBatchOutput = std::vector<typename Types::ModelOutput>;
        using ModelBatchInput = std::vector<_State>;
        using ModelInput = _State;
    };

    typename Types::PRNG device;

    MonteCarloModel(typename Types::PRNG &device) : device(device) {}

    void get_input(
        const typename Types::State &state,
        typename Types::ModelInput &input)
    {
        input = state;
    }

    void get_batch_input(
        const std::vector<typename Types::State> &states,
        typename Types::ModelBatchInput &inputs)
    {
        inputs = states;
    }

    void get_inference(
        typename Types::ModelInput &input,
        typename Types::ModelOutput &output)
    {
        const typename Types::Real row_uniform{1 / (typename Types::Real)input.row_actions.size()};
        output.row_policy.fill(input.row_actions.size(), row_uniform);
        const typename Types::Real col_uniform{1 / (typename Types::Real)input.col_actions.size()};
        output.col_policy.fill(input.col_actions.size(), col_uniform);

        rollout(input);
        output.row_value = input.row_payoff;
        output.col_value = input.col_payoff;
    }

protected:
    void rollout(_State &state)
    {
        // model inference in bandits happens in expand(), after get_actions is called
        while (!state.is_terminal)
        {
            const ActionIndex row_idx = device.random_int(state.row_actions.size());
            const ActionIndex col_idx = device.random_int(state.col_actions.size());
            const typename Types::Action row_action = state.row_actions[row_idx];
            const typename Types::Action col_action = state.col_actions[col_idx];
            state.apply_actions(row_action, col_action);
            state.get_actions();
        }
    }
};